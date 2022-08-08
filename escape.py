import html
import sys

print(html.escape(open(sys.argv[1], 'r').read()), end='')
