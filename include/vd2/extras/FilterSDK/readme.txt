There are the header files that were included with the last version of the
Filter SDK. You can use these if you need to rebuild a filter that doesn't
compile with the new headers, or otherwise need to know the full API supported
by that SDK (such as if you are building a new filter host). If possible,
you should use the migration headers in include/vd2/OldFilterSDK instead,
and new filters should use <vd2/plugin/vdvideofilt.h> directly.
