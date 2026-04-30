# Star Extractor

Whilst I can use/wrap the command line tool *sex* - I would like to see how difficult this is using C++.

Whilst not trivial, it is not too complex once you can figure out the construction of the C based data calls.

## Logic

Load Fits file (slow)
The have 2 seperate routines where the params can be altered without having to reload the FITS file

- extractBackground
- extractStars

We can change parameters in each section and then only process the latter sections.

i.e. if we think we have the background calculated - we just adjust the Star extraction params and run that section.
