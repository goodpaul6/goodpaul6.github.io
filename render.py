# Loops through all folders in the current directory except for
# the 'docs' folder, renders the markdown files they contain.
from typing import List

import os
import dotenv
import dataclasses
import datetime
import re
import time
import logging
import sys
import markdown2

dotenv.load_dotenv()

OUTPUT_DIR = "docs"
POST_FILENAME = "README.md"
INCLUDE_DIRECTIVE = re.compile(r'^%include "(.*?)"$')
UPDATE_EVERY_SECONDS = 3
INDEX_FILENAME = "index.html"

CODE_EXT_TO_LANGUAGE = {"cpp": "cpp", "hpp": "cpp", "h": "c", "json": "json"}


@dataclasses.dataclass
class Post:
    path: str
    output_path: str
    title: str
    date: str
    markdown_text: str


def render_text(text: str) -> str:
    html = markdown2.markdown(text, extras=["fenced-code-blocks"])

    return (
        f"""
    <head>
        <link rel="stylesheet" href="/styles.css">
        <link rel="stylesheet" href="/monokai.css">
    </head>
    """
        + html
    )


def create_post_from_file(path: str) -> str:
    # Posts are markdown files
    assert path.endswith(".md")

    logging.debug(f"Creating post for {path}")

    with open(path, "r") as f:
        title = None
        date = None
        markdown_lines = []

        for line in f.read().splitlines():
            if not title and line.startswith("# "):
                title = line[2:]

                markdown_lines.append(line)
            elif not date and line.startswith("_20"):
                date = line[1:-1]

                markdown_lines.append(line)
            elif m := INCLUDE_DIRECTIVE.match(line):
                include_name = m.group(1)

                ext = os.path.splitext(include_name)[1]

                # Right now only code languages are supported
                # by include.
                lang = CODE_EXT_TO_LANGUAGE[ext[1:]]

                markdown_lines.append(f"```{lang}")

                include_path = os.path.join(os.path.dirname(path), include_name)

                logging.debug(f"Including {include_path} into {path}")

                markdown_lines.extend(line for line in open(include_path, "r"))

                markdown_lines.append("```")
            else:
                markdown_lines.append(line)

        logging.debug(f"Created post {title} - {date}")

        return Post(
            path=path,
            title=title,
            date=date,
            markdown_text="\n".join(markdown_lines),
            output_path=os.path.join(
                OUTPUT_DIR, path.replace(POST_FILENAME, INDEX_FILENAME)
            ),
        )


def collect_posts(modified_only) -> List[Post]:
    posts: List[Post] = []

    logging.info("Scanning for posts...")

    # Posts are directories which have a README.md.
    # We only go a single level deep because I don't
    # like deep nesting.
    for entry in os.scandir():
        if entry.name == OUTPUT_DIR:
            continue

        if entry.is_dir():
            readme_path = os.path.join(entry.path, POST_FILENAME)

            if not os.path.exists(readme_path):
                continue

            if modified_only and os.stat(readme_path).st_mtime <= os.stat(
                os.path.join(OUTPUT_DIR, readme_path).st_mtime
            ):
                continue

            logging.debug(f"Found post at {readme_path}")

            posts.append(create_post_from_file(readme_path))

    logging.info(f"Collected {len(posts)} posts.")

    return posts


def render_posts(posts: List[Post]):
    logging.info("Rendering posts...")

    for post in posts:
        html = render_text(post.markdown_text)

        logging.debug(f"Rendered {post.path} to HTML.")

        os.makedirs(os.path.dirname(post.output_path), exist_ok=True)

        with open(post.output_path, "w") as f:
            f.write(html)

    logging.info("Done rendering posts.")


def render_index(posts: List[Post]):
    logging.info("Rendering index file...")

    markdown_lines = [
        "# Apaar Madan",
        "",
        "I do tech stuff at [PostGrid](https://postgrid.com). I also make games, game engines, and compilers.",
        "You can find some of that code [here](https://github.com/goodpaul6). This is my personal blog.",
        "",
        "## Archive",
        "",
    ]

    for post in posts:
        markdown_lines.append(
            f"* [{post.title}]({post.output_path[len(OUTPUT_DIR) + 1:]}) - _{post.date}_"
        )

    html = render_text("\n".join(markdown_lines))

    logging.debug("Converted index markdown to HTML.")

    with open(os.path.join(OUTPUT_DIR, INDEX_FILENAME), "w") as f:
        f.write(html)

    logging.info("Wrote index file.")


def render_blog(watch):
    while True:
        posts = collect_posts(watch)

        render_posts(posts)
        render_index(posts)

        if not watch:
            break

        logging.debug(f"Sleeping for {UPDATE_EVERY_SECONDS} seconds...")

        time.sleep(UPDATE_EVERY_SECONDS)


def main():
    watch = len(sys.argv) > 1 and sys.argv[1] == "watch"

    logging.info(f"Rendering with watch={watch}")

    render_blog(watch)


if __name__ == "__main__":
    main()
