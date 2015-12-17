# compress
An implementation of a simple arithmetic coding data compression algorithm.

Supports basic arithmetic coding, PPM (prediction by partial matching) and BWT (Burrows–Wheeler transform) + MTF (move-to-front) + basic arithmetic.

You can build it on both Windows and Linux.

```
Usage:
	compress c input output ari - basic arithmetic
	compress c input output ppm - PPM
	compress c input output bwt - BWT + MTF + basic
	compress d output output_dec - decompression (works for all three types)
```
