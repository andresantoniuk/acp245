#!/usr/bin/python
import sys
from acpyd.scriptdoc import HtmlGenDoc, DocParser

scripts_dir = sys.argv[1]
docfile = sys.argv[2]
cmds = DocParser().parse(scripts_dir)

f = open(docfile, 'w')
f.write(HtmlGenDoc().gen(cmds))
f.close()
