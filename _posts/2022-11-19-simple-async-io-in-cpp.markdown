---
layout: post
title: 'Simple Async IO in C++17'
date: 2022-11-19 18:28:00 -0500
author: Apaar Madan
categories: cpp
---

I feel like there are enough blog posts on the internet explaining the utility
of async IO, so won't discuss that here. That being said, I enjoyed setting up
this system from scratch for my in-memory database project
[boutique](https://github.com/goodpaul6/boutique).

## An Example Using Our API

Here's an echo server created using the asio system we'll be building over the
course of this post. By the way, this isn't blogware, you can grab the full
source
[here](https://github.com/goodpaul6/goodpaul6.github.io/tree/main/_includes/asyncio).

{% highlight cpp %} {% include asyncio/example.cpp %} {% endhighlight %}

## Why Not Use [ASIO](https://think-async.com/Asio)

First of all, ASIO is a fantastic library, and you can see a lot of similarities
between its API and the example above. However, as is the case with a lot of
projects I build, I always find value in writing simple code that does exactly
what I need; no more, no less.

The first advantage is debuggability. It is very easy to reason about what is
going on underneath the hood since everything is approximately one layer deep
and there are only a handful of files. Thanks to ASIO, we already have an
approximation for what a useful async IO API looks like, making the
implementation much more focused.

The second advantage is compile times. Naturally, the footprint of what we're
building here is much smaller than that of ASIO. More importantly, we don't use
any template metaprogramming since it is simply not needed for the capabilities
we're providing here.

The third is that it is fun. I wrote this code for
[Boutique](https://github.com/goodpaul6/boutique) and one of the goals of that
project is to avoid external dependencies to explore all the details involved in
building a production-ready in-memory database (_Maybe even use it for some of
our internal services at [PostGrid](https://postgrid.com)_). This is _arguably_
a lot more fun if we're spending the majority of the time reasoning about
interesting problems like "How do I solve my specific problem?" as opposed to
"How can I glue this networking library with this storage library?"
