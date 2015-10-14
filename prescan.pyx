cdef extern from "prescan.h":
    const char *prescan_a_byte_stream_to_determine_its_encoding(
        const char *buffer, size_t nbytes);

def determine_encoding(text):
    cdef bytes block = text[:1024]
    cdef const char *rv

    rv = prescan_a_byte_stream_to_determine_its_encoding(block, len(block))
    if not rv: return None
    return (<bytes>rv).decode("ascii")
