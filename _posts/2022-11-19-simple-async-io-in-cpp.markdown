---
layout: post
title: 'Simple Async IO in C++17'
date: 2022-11-19 18:28:00 -0500
author: Apaar Madan
categories: jekyll update
---

I feel like there are enough blog posts on the internet explaining the utility of async IO, so
won't discuss that here. That being said, I enjoyed setting up this system from scratch
for my in-memory database project [boutique](https://github.com/goodpaul6/boutique).

## An Example Using Our API

Here's an echo server created using the asio system we'll be building over the course of this post.

{% highlight cpp %}
{% include asyncio/example.cpp %}
{% endhighlight %}
