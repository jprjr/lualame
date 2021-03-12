# lualame

Lua bindings to [libmp3lame](https://lame.sourceforge.io/).

MIT licensed (see file `LICENSE`).

Currently covers the encoding API, but not the decoding API.

# Installation

## Building from source

You can build with luarocks or cmake.

# Table of Contents

# Synopsis

```lua
-- demo program that allocates an encoder, sets
-- some tags, encodes some samples, and writes
-- out a file

-- we're going to assume we've got a table of
-- audio samples, interleaved like
-- { left_channel, right_channel, ... }
-- each sample is an integer, in the signed,
-- 16-bit range.
local samples = {} -- pretend this has stuff

local lame = require'lualame'
local encoder = lame.lame_init()

-- use average bitrate vbr
lame.lame_set_VBR(encoder,lame.vbr_abr)

-- encoder has a metatable set to allow object-oriented use
encoder:set_in_samplerate(44100) -- equivalent to lame_set_in_samplerate(encoder,44100)
encoder:set_num_channels(2) -- equivalent to lame_set_num_channels(encoder,2)

encoder:id3tag_init()
encoder:id3tag_v2_only(true)
encoder:id3tag_set_title("Hot New Song")
encoder:id3tag_set_artist("That Cool New Artist")
encoder:id3tag_set_album("Who buys albums anymore?")

-- make sure we're good to go
assert(encoder:init_params())

local output = io.open('output.mp3','wb')
output:write(encoder:encode_buffer_interleaved(samples))
output:write(encoder:encode_flush())
output:close()

```

# Functions

I use the same function names, enum names, etc as `libmp3lame`, you should be
able to follow along by reading lame's C library header.

Here's some important ones:

## lame_init

**syntax:** `userdata state = lame.lame_init()`

Allocates a new encoder instance. The instance has a metatable set,
allowing for easy object-oriented usage.

Basically, take any function, remove the `lame_` prefix, and you've
got the metatable version. Example:

`lame_init_params(state)` can be written as `state:init_params()`


