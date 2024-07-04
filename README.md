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

Internal changes:
* There is no longer a internal::measure or internal::compare; these just use the std::char_traits<Ch> functions as used by the string_views.

### Gotchas

The functions like find_node and name(...) that took a Ch * and optional length now take only a
std::basic_string_view<Ch>. Typical usage passed in 0, NULL, or nullptr for unwanted values; this will now segfault on C++20
and earlier - use C++23 ideally, but you can pass in {} instead. This should probably be a
std::optional<std::basic_string_view<Ch>> instead.

## Changes

I needed a library for fast XMPP processing (reading, processing, and reserializing), and this mostly fit the bill. However, not entirely, so this version adds:

* XML Namespace support
* An additional parse mode flag for doing shallow parsing.
* An additional parse mode flag for extracting just one (child) element.

## Tests

The other thing this fork added was a file of simple tests, which I've recently rewritten into GoogleTest.

The original makes reference to an expansive test suite, but this was not included in the open source release. I'll expand these tests as and when I need to.

## Pull Requests

Erm. I didn't expect any, so never set up any of the infrastructure for them - this was really a fork-of-convenience for me. Not that they're unwelcome, of course, just entirely unexpected.

But yeah, go for it, just include an assurance you're happy with the licensing.