# Game Texture Loader Version 1

### About

This is version 1 of a loader for *game format* texture loader.

The code dates from around 2005 and was written at a time when, in my opinion, the various common texture loading libraries in use were doing things I didn't like; either the interface wasn't what I wanted or the library did something dumb (FreeImage, for example, would decompress DXT images which was precisely what you didn't want to happen.)

This is the first verison of the library somewhat frozen in time.

### Core Concepts

The library depends heavily of Boost, namely the file streaming library there in, as the decoders/loaders are totally based around that concept.

The system was setup in such a way as loading sources were based on 'devices' (File and PhysFS support being present) and the various decoding routines were filters.

The filters abused static objects to register themselves with a Boost multi-index map, which was used to find a loading *filter* by either file extension or ID. 

There was also support for user defined filters which could be setup at run time.

### Notes

Ultimately while this library worked I gave up on the boost integration/iostreams as a concept and moved on to a second version (see other repos).

This library depends on libpng, zlib and libjpg to build - none of which are included.

Has Visual Studio 2003 and 2005 solutions.

You probably shouldn't use this code as is for anything, but it is here for historical sake.

Licensed under MIT - see the license text for details.