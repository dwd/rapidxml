# RapidXML
## Dave's Version

Hey! This is RapidXML, an ancient C++ library for parsing XML quickly and flexibly.

There's a lot of forks of this around, and I (Dave Cridland) didn't write the vast majority of this library - instead, it was written by someone called Marcin Kalicinski, and his copyright is dated 2009.

## Version 2, Breaking Changes

This is version 2.x. You might not want this.

It has breaking changes, the largest of which are:
* No more case insensitive option. Really, nobody should be using XML case insensitively anyway, but it was too difficult to keep around, sorry.
* Instead of passing around potentially unterminated character pointers with optional lengths, we now use std::basic_string_view
* There is no need for string termination, now, so the parse function never terminates, and that option has vanished.
* Return values that were previously bare pointers are now a safe wrapped pointer which ordinarily will check/throw for nullptr.
* append/prepend/insert_node now also have an append/prepend/insert_element shorthand, which will allow an XML namespace to be included if wanted.
* Parsing data can be done from a container as well as a NUL-terminated buffer. A NUL-terminated buffer remains slightly faster, and will be used if possible (for example, if you pass ina  std::basic_string, it'll call c_str() on it and do that).

Not breaking, but kind of nice:
* The parse buffer is now treated as const, and will never be mutated. This incurs a slight performance penalty for handling long text values that have an encoded entity late in the string.
* The iterators library is now included by default, and updated to m_handle most idiomatic modern C++ operations.

Internal changes:
* There is no longer a internal::measure or internal::compare; these just use the std::char_traits<Ch> functions as used by the string_views.
* Reserialization (that is, using the rapidxml::print family on a tree that is mostly or entirely from parsing) is now much faster, and will optimize itself to use simple buffer copies where the data is unchanged from parsing.
* Alignment of the allocator uses C++11's alignof/std::align, and so should be more portable.

New features:
* Instead of the `doc->allocate_node` / `node->append_node` dance, you can now `node->append_element(name, value)`, where `name` can be either a `string` (or `string_view`, etc) or a tuple like {xmlns, local_name}, which will set an xmlns attribute if needed.
* There's a xpathish thing going on in `rapidxml_predicates`, which lets you search for (or iterate through) elements using a trivial subset of XPath.
* You can get access to containerish things in rapidxml_iterators by methods on nodes/documents, as `node.children()`, `node.attributes()` and a new `node.descendants()`.

### Fun

The rapidxml_iterators library is now included in rapidxml.hpp, and you can do amusing things like:

```c++
for (auto & child : node.children()) {
    if (child.name() == "potato") scream_for(joy);
}
```

More in [test/iterators.cpp](./test/iterators.cpp)

Of course, in this case it might be simpler to:

```c++
auto xpath = rapidxml::xpath::parse("/potato");
for (auto & child : xp->all(node)) {
    scream_for(joy);
}
```

More of that in [test/xpath.cpp](./test/xpath.cpp)

For those of us who lose track of the buffer sometimes, clone_node() now takes an optional second argument of "true" if you want to also clone the strings. Otherwise, nodes will use string_views which reference the original parsed buffer.

### Gotchas

The functions like find_node and name(...) that took a Ch * and optional length now take only a
std::basic_string_view<Ch>. Typical usage passed in 0, NULL, or nullptr for unwanted values; this will now segfault on C++20
and earlier - use C++23 ideally, but you can pass in {} instead. This should probably be a
std::optional<std::basic_string_view<Ch>> instead.

## Changes to the original

I needed a library for fast XMPP processing (reading, processing, and reserializing), and this mostly fit the bill. However, not entirely, so this version adds:

* XML Namespace support
* An additional parse mode flag for doing shallow parsing.
* An additional parse mode flag for extracting just one (child) element.

## Tests

The other thing this fork added was a file of simple tests, which I've recently rewritten into GoogleTest.

The original makes reference to an expansive test suite, but this was not included in the open source release. I'll expand these tests as and when I need to.

The tests use a driver which can optionally use Sentry for performance/error tracking; to enable, use the CMake option RAPIDXML_SENTRY, and clone the [sentry-native](https://github.com/getsentry/sentry-native) repository into the root, and when running `rapidxml-test`, set SENTRY_DSN in the environment. None of the submodules are needed, but it'll need libcurl, so `sudo apt install libcurl4-openssl-dev`.

## Pull Requests

Erm. I didn't expect any, so never set up any of the infrastructure for them - this was really a fork-of-convenience for me. Not that they're unwelcome, of course, just entirely unexpected.

But yeah, go for it, just include an assurance you're happy with the licensing.