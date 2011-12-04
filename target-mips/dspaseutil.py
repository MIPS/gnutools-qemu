import re

# simple for now. just take care of "."
# should really replace all non c characters with _
#
def make_legal_c(n):
    p = re.compile("\.");
    n = p.sub('_', n);
    p = re.compile("\[")
    n = p.sub("l_", n)
    p = re.compile("\]")
    n = p.sub("_r", n)
    return n

def contains_alternate(n):
    return re.search("\[", n)

# can only have one []
#
def alternate_1(n):
    return re.sub(r'\[.+\]', '', n)


def alternate_2(n):
    return re.sub(r'\]', '', re.sub(r'\[', '', n))

