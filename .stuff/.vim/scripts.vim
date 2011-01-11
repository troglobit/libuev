" my filetype file
if did_filetype()
  finish
endif
if getline(1) =~ '^#!/bin/d\=ash'
    call SetFileTypeSH("ksh")	" defined in filetype.vim
endif
