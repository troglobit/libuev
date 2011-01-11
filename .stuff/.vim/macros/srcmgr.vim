" Author: fma fro Ron Aarons newfmgr.vim

let g:srcsmgrhelp= "|'o'pen, 'e'dit or 'r'eplace to edit\n|Common listing for: "
	
" Set up hook to Common buffers:
if 1
aug srcsmgr
	au!
	au BufReadPre,BufEnter CMN::*/* nested call SrcmgrReload(expand("%"))
	au BufReadPre,BufEnter CMN::*   nested call SrcmgrReload(expand("%"))
	au BufReadPre,BufEnter GRP::*   nested call SrcmgrReload(expand("%"))
	au BufReadPre,BufEnter GLM::*   nested call SrcmgrReload(expand("%"))
	au BufReadPre,BufEnter GLW::*   nested call SrcmgrReload(expand("%"))
aug END
endif

" This routine gets called when we 'edit' a directory
func! SrcmgrReload(name)
	let g:vimedname = a:name
	if ! exists("b:last") || (line("$")<=b:last)
		call ResultsWindow('-NEW-', g:srcsmgrhelp.a:name, 'call SrcmgrInit()',
							  \ 'SrcmgrOnClick', 0, 'SrcmgrMap')
	endif
endfunc

" This one gets called by the ResultsWindow routine on initializing the window
func! SrcmgrInit()
	let b:vimedname = g:vimedname
	if exists("b:last") && (line("$")>b:last)
		exe (b:last+1).',$ delete _'
	endif
	if exists("g:CommonRes")
		exe "put=g:CommonRes"
		unlet g:CommonRes
	else
		if     b:vimedname =~ "CMN::" | let Command = "Common "
		elseif b:vimedname =~ "GRP::" | let Command = "Grep "
		elseif b:vimedname =~ "GLM::" || b:vimedname =~ "GLW::"
		   if exists("g:GlimpsePath")
		      let Command = "glimpse-tt -H ".g:GlimpsePath." "
		   else
		      let Command = "Glimpse "
		   endif
		   if b:vimedname =~ "GLW::" 
		      let Command = Command."-w "
		   endif
		endif
		exe "put=system(\\\"".Command."'".strpart(b:vimedname, 5, 1000)."'\\\")"
	endif
	set nomodified
	syntax match MatchedPart /:.*/
	syntax match PathPart1 "/[^/]*$"ms=s+1 contained
	syntax match PathPart2 "/[^/:]*:"me=e-1,ms=s+1 contained
	syntax match LeadPath "^/[^:]*"ms=s+1 contains=PathPart1,PathPart2
	hi link MatchedPart Question
	hi link PathPart1 Title
	hi link PathPart2 Title

	syntax match Hide /^|/ contained
	syntax match Lead /^|.*/ contains=Hide
	hi link Lead Directory
	hi link Hide NonText
endfunc

" as does this, by ENTER or Dbl-click
func! SrcmgrOnClick(line)
	call SrcmgrApply('e!')
endfunc


func! SrcmgrApply(what)
	" get the file on which the line is
	let path = substitute(getline("."), ":.*", '', '')
	if path !~ '^\/' 
		let path = g:GlimpsePath.'/'.path
	endif
	if ! filereadable(path) && ! isdirectory(path)
		return
	endif
	if b:vimedname !~ '^CMN::'
		if &hlsearch == 0
			aug srcsmgr
				exe "au BufLeave ".path." set nohlsearch | au! srcsmgr BufLeave"
			aug END
		endif
		set hlsearch
		let @/ = strpart(b:vimedname, 5, 1000)
	else
		let @/ = "."
	endif
	if (a:what == 'new!')	" create new window for file
		exe 'new +/'.@/.' '.path
	elseif (a:what =~ 'e!') " just edit the file
		set nomodified
		if a:what == 'e!!'
			exe "bdelete ".bufnr("%")
		endif
		exe 'find +/'.@/.' '.path
	else
		echo 'SrcmgrApply: '.a:what.']'
	endif
	set readonly
endfunc

" turn special mappings on or off:
func! SrcmgrMap(on)
	if a:on
		nmap o :call SrcmgrApply('new!')<cr>
		nmap e :call SrcmgrApply('e!')<cr>
		nmap r :call SrcmgrApply('e!!')<cr>
	else
		unmap o
		unmap e
		unmap r
	endif
endfunc
