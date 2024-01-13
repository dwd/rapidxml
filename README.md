# RapidXML
## Dave's Version

Hey! This is RapidXML, an ancient C++ library for parsing XML quickly and flexibly.

There's a lot of forks of this around, and I (Dave Cridland) didn't write the vast majority of this library - instead, it was written by someone called Marcin Kalicinski, and his copyright is dated 2009.

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