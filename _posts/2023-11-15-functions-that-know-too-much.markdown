---
layout: post
title: 'Functions That Know Too Much'
date: 2023-11-15 11:33:00 -0500
author: Apaar Madan
published: true
categories: style typescript
---

I try to avoid writing functions that know too much. What is a "function that
knows too much"? If I had to write a Platonic definition, it's a function which
concerns itself too much with the accidental[^1] qualities of the problem it is
trying to solve.

Let's work through an example

```ts
// template.ts
interface Template {
    description: string;
    user: string;
    html: string;
    createdAt: Date;
    updatedAt: Date;
}

// utils.ts
import { Template } from './templates';

const escapeCurlyBraces = (template: Template): void => {
    template.html = template.html.replace(/\{/g, '\\{').replace(/\}/g, '\\}');
};
```

We can see that this function operates exclusively on the `html` string
contained within the `Template` interface, but it requires a `Template` to be
passed in order to be used. This is not ideal; not only is it a hindrance for
reuse/testing (we'll probably need to escape curly braces in contexts where we
don't have a template), since this operates in-place, it's unclear to the caller
what other fields of `Template` get modified.

This is easily remedied. Just write a function that receives a string and
returns a string.

```ts
const escapeCurlyBraces = (str: string): string => {
    return str.replace(/\{/g, '\\{').replace(/\}/g, '\\}');
};
```

and if we find ourselves doing
`template.html = escapeCurlyBraces(template.html)` frequently, we can add the
following function

```ts
const escapeTemplateCurlyBracesInPlace = (template: Template): void => {
    template.html = escapeCurlyBraces(template.html);
};
```

Now this function clearly acknowledges that it's operating on the template in
place and we still have access to the underlying `escapeCurlyBraces` whenever we
need it.

Let's look at a less obvious example

```ts
const MAX_REGEX_CHARS = 10_000;

// PROBLEM:
// Given an array of ID prefixes,
// create regexes that match any of the given ID prefixes.
// Each regex can be at most 10K characters long, so split up the
// batches appropriately.

const batchIDPrefixRegexes = (idPrefixes: string[]): RegExp[] => {
    const res: RegExp[] = [];

    let curLength = 0;
    const curParts: string[] = [];

    const flush = () => {
        if (curParts.length === 0) {
            return;
        }

        // Since these are just prefixes, we want to match from the start
        // and use a non-capturing group.
        res.push(new RegExp(curParts.map((v) => `(?:^${v})`).join('|')));

        curLength = 0;
    };

    for (const [index, id] of idPrefixes.entries()) {
        if (curLength + id.length >= MAX_REGEX_CHARS) {
            flush();
        }

        curParts.push(id);

        // The 4 is because of the parens and non-capturing
        // specifier. The 5 is with the pipe included.
        const additionalChars = index === ids.length ? 4 : 5;

        curLength += additionalChars + id.length;
    }

    flush();

    return res;
};
```

Let's enumerate everything this function knows:

-   The caller is supplying ID prefixes, so we want to match with the start of
    an ID
-   They want to use non-capturing groups
-   Each batch regex can only have `MAX_REGEX_CHARS` characters
-   They want to receive regexes as a result

As it stands, this function is perfectly fine. It solves the problem it's trying
to solve, it's easy to use, and it's readable. It's important to remember that
this is our goal, not writing ostensibly reusable code.

That being said, let's say that we now have a requirement to perform
case-insensitive matching on complete names instead of IDs in some cases. Our
first instinct might be to add a parameter to distinguish the two:

```ts
const batchRegexes = (inputs: string[], idPrefixes: boolean): RegExp[] => {
    // Same as above...

    const flush = () => {
        if (curParts.length === 0) {
            return;
        }

        if (idPrefixes) {
            res.push(new RegExp(curParts.map((v) => `(?:^${v})`).join('|')));
        } else {
            res.push(
                new RegExp(curParts.map((v) => `(?:^${v}\$)`).join('|'), 'i')
            );
        }

        curLength = 0;
    };

    for (const [index, id] of inputs.entries()) {
        // Same as above...

        const additionalChars =
            (idPrefixes ? 5 : 4) + (index < inputs.length - 1 ? 1 : 0);

        // Same as above...
    }
};
```

But now we have a function that masquerades as something general but is actually
just solving two very specific problems in an inflexible way. Moreover, the
caller now has to divine what the difference in behavior is when they pass along
`false` for `idPrefixes`.

How can we improve this? Let's zoom in on the branch

```ts
if (idPrefixes) {
    res.push(new RegExp(curParts.map((v) => `(?:^${v})`).join('|')));
} else {
    res.push(new RegExp(curParts.map((v) => `(?:^${v}\$)`).join('|'), 'i'));
}
```

There are two major additions in the `else` branch:

-   The regex pattern matches the end of the string as well via `\$`
-   An option is specified to make the regex case insensitive

If we look back at our prior list of what the function knows, these relate to:

-   The caller is supplying ID prefixes, so we want to match with the start of
    an ID
-   They want to use non-capturing groups
-   They want to receive regexes as a result

Seeing as how changing the requirement slightly invalidated most of these
points, we can make an educated guess that what really matters is the batching
aspect of this function.

Let's rewrite it accordingly:

```ts
const batchRegexPatterns = (patterns: string[]): string[] => {
    // Same as above...

    const flush = () => {
        if (curParts.length === 0) {
            return;
        }

        res.push(curParts.map((v) => `(${v})`).join('|'));

        curLength = 0;
    };

    for (const [index, id] of ids.entries()) {
        // Same as above...

        // Now we just account for the parens and pipe
        const additionalChars = index === ids.length - 1 ? 2 : 3;

        // Same as above...
    }
};
```

At a glance, it just looks like we got rid of key functionality; that's because
we did. The function no longer takes into account whether we're working with ID
prefixes, or some other type of pattern. And that's the point.

Now, if somebody wanted to use it for ID prefixes, they can do so as follows:

```ts
const idRegexes = batchRegexPatterns(
    idPrefixes.map((prefix) => `?:^${prefix}`)
).map((pattern) => new RegExp(pattern));
```

And as follows for case-insensitive exact matches on names:

```ts
const nameRegexes = batchRegexPatterns(names.map((name) => `?:^${name}`)).map(
    (pattern) => new RegExp(pattern, 'i')
);
```

So what does our new function know?

-   Each batch regex can only have `MAX_REGEX_CHARS` characters
-   The caller wants to receive strings with the regex patterns as a result

With these assumptions in place, the function is both simpler and more flexible,
and the calling code is more explicit about its behaviour.

Let's look at another quick example.

```ts
const createWidget = async (params: CreateWidgetParams) => {
    // You can imagine we're doing a bunch of database round trips here.
    await validateCreateWidgetParams(params);
    return await db.create(params);
};
```

Again, this is seemingly innocuous, but there's one assumption that jumps out at
me: we're only going to be creating one widget at a time. In my experience, this
is rarely the case.

This is a function that knows too much. Assuming that we'll only be creating one
widget at a time is a _stronger_ assumption than assuming we'll create any
number of them. Sure, you can call this function in a loop, or even wrap many
concurrent invocations in a `Promise.all`, but you'll be paying the cost of the
database round trips for _each one_. We can argue about the elegance of our
`batchRegexPatterns` function above all day, but we can't argue the relatively
large performance hit we take with `createWidget` here for batch use cases.

## Conclusion

It pays to evaluate what assumptions your code makes and see if you can modify
them to fit your problem better. Doing this in a code review setting is
particularly effective. Over time, you get a feel for the right assumptions and
you can spend less time refactoring and more time getting stuff done.

[^1]: https://plato.stanford.edu/entries/essential-accidental/
