---
layout: post
title: 'Simple Async IO in C++17'
date: 2023-01-19 01:15:00 -0500
author: Apaar Madan
published: false
categories: cpp
---

I feel like there are enough blog posts on the internet explaining the utility
of async IO, so won't discuss that here. Let's start by addressing the elephant
in the room.

## Why Not Use [ASIO](https://think-async.com/Asio)?

First of all, ASIO is a fantastic library, and you can see a lot of similarities
between its API and the one we'll build here. However, as is the case with a lot
of projects I build, I always find value in writing simple code that does
exactly what I need; no more, no less.

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
building an in-memory database. This is _arguably_ a lot more fun if we're
spending the majority of the time reasoning about interesting problems like "How
do I solve my specific problem?" as opposed to "How can I glue this networking
library with this storage library?"

## An Example Using Our API

Here's an echo server created using the asio system we'll be building over the
course of this post. By the way, this isn't blogware, you can grab the full
source
[here](https://github.com/goodpaul6/goodpaul6.github.io/tree/main/_includes/asyncio).

<!-- prettier-ignore -->
{% highlight cpp %} 
{% include asyncio/example.cpp %} 
{% endhighlight %}

## Creating A Socket

We'll start by creating a TCP socket abstraction that can work nicely with the
rest of our infrastructure. It is a very thin wrapper over BSD sockets, so it
can be used for both synchronous and asynchronous IO (demonstrated in the
example above).

Let's take a look at `socket.hpp`

<!-- prettier-ignore -->
{% highlight cpp %} 
{% include asyncio/socket.hpp %} 
{% endhighlight %}

You can see that we have a pretty barebones API, but all we really care about is
being able to `send` and `recv` and do so without blocking. Note that we can
also modify whether operations block using `Socket::set_non_blocking`. This is
useful for creating simple synchronous clients like the one in our example
above.

You can take a look at the implementation
[here](https://github.com/goodpaul6/goodpaul6.github.io/tree/main/_includes/asyncio/socket.cpp).
The only thing of note is the copious use of the following helper

<!-- prettier-ignore -->
{% highlight cpp %} 
void throw_errno(const char* what) {
    throw std::system_error{make_error_code(static_cast<std::errc>(errno)),
                            what};
}
{% endhighlight %}
