" pyclewn run time file
" Maintainer:   <xdegaye at users dot sourceforge dot net>
"
" Configure VIM to be used with pyclewn and netBeans
"
" $Id: pyclewn.vim 16 2008-06-06 19:08:47Z xdegaye $

if exists("b:did_pyclewn")
    finish
endif
let b:did_pyclewn = 1

" pyclewn version
let g:pyclewn_version = "pyclewn-0.5"

" enable balloon_eval
if has("balloon_eval")
    set ballooneval
    set balloondelay=100
endif



