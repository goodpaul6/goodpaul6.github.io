# Loops through all folders in the current directory except for
# the 'docs' folder, renders the markdown files they contain.
from typing import List

import os
import dotenv
import requests
import dataclasses
import datetime

dotenv.load_dotenv()

GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")
GITHUB_CONTEXT = os.getenv("GITHUB_CONTEXT")

OUTPUT_DIR = "docs"

@dataclasses.dataclass
class Post:
    title: str
    date: str
    markdown_text: str

@dataclasses.dataclass
class Blog:
    posts: List[Post]


def render_text(text: str) -> str:
    res = requests.post(
        "https://api.github.com/markdown",
        headers={"Authorization": f"Bearer {GITHUB_TOKEN}"},
        json={"text": text, "mode": "gfm", "context": GITHUB_CONTEXT},
    )

    return res.text


def create_post_from_file(path: str) -> str:
    # Posts are markdown files
    assert path.endswith('.md')

    with open(file, 'r') as f:
        markdown_text = f.read()


def collect_posts() -> List[Post]:
    # We only go a single level deep because I don't
    # like deep nesting.
    for entry in os.scandir():
        if entry.is_dir():



def main():
    print(render_text(input()))


if __name__ == "__main__":
    main()
