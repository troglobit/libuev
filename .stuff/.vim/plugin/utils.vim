" collection of misc code I find useful
" Author: Ron Aaron ron@mossbayeng.com
"
" 'if 0'ed' everything outside the StatusWindow stuff /fma
"
" last update: Sun Jul 11 14:42:38 1999
if 0
set nocp

let g:utils_package_loaded = 1

" define whatever this package does, here
func! SetIfBlank(var, value)
	let value = a:value
	if !exists(a:var)
		if match(a:value, '^expand') >= 0
			" expand it
			exe 'let value=' a:value
			let value=escape(value, '\')
		endif
		exe 'let ' a:var '= "'. value .'"'
	endif
endfunc
command! -nargs=* SetIfBlank call SetIfBlank(<f-args>)

" convert a number or bool to a portion of the passed-in string
func! ValueToString(choices, value, bool)
	let retval = '<unknown>'
	
	" first, convert the 'value' to an index:
	if (a:bool)
		" boolean.  0 == 'n' == 'No' == 'false'
		if a:value=~"[a-zA-Z]"
			if ((a:value =~ "^[nN]o*") || (a:value=~"^[Ff][aA][lL][sS][eE]"))
				let index = 0
			else
				let index = 1
			endif
		else
			let index = !(a:value == 0)
		endif
	else
		" force numeric.
		let index = 0 + a:value
	endif

	" second, find the 'index'th string in 'choices', which should be a
	" comma-delmited string
	let x=0
	let choice=a:choices
	while 1
		if (x == index)	" that's our boy!
			let ix = match(choice, ',')
			" extract the string up till the comma
			if ix == -1
				let ix = 99999
			endif
			let retval = strpart(choice, 0, ix)
			return retval
		endif
		" get next string part
		let ix = match(choice, ',')
		if ix == -1
			" failed!
			return retval
		endif
		let choice = strpart(choice, ix+1, 99999)
		" bump counter
		let x = x+1
	endwhile	
endfunc

" convert a number or bool to a portion of the passed-in string
func! BoolString(bool)
	return ValueToString('no,yes', a:bool, 1)
endfunc

endif

func! Fnmunge(name)
	let name = substitute(a:name,'[\\/ \t:]', '', 'g')
	return name
endfunc

func! On_(true)
	if a:true | return "no" | else | return "" | endif
endfunc

func! ResultsWindow (name,help,cmd,action,once,maps)
	" turn off reporting
	let rep=&report | let mo=On_(&more) | let ch=&ch | let te=On_(&terse)
	set report=9999 nomore ch=3 hidden terse
	
	let retval=0
	
   if exists("b:origbuf")
      let origbuf = b:origbuf
   elseif a:name == '-NEW-'
      let origbuf = bufnr(@#)
   else
      let origbuf = bufnr("")
   endif

   let b:samebuf=0
	if (a:name == '')
		let b:samebuf=1
		% d _
   elseif a:name == '-NEW-'
		% d _
	else
		let name=g:home.'/'.Fnmunge(a:name)
		exe 'new ' . name
	endif
   let b:origbuf = origbuf
	let b:resultswindow=1
	let b:action = a:action
	let b:once = a:once
	let b:maps = a:maps		" additional map function (one parm: 1=on, 0=off)

	0 put=a:help
	let b:last = line("$")	" save last line number
	normal G

	" fill in the buffer with something:
	exe a:cmd
	if v:shell_error < 0
		if !b:samebuf
			unlet b:resultswindow
			bdelete!
		endif
		echo 'Problem executing command: ' . a:cmd . ':' . v:errmsg
		return
	endif

	let last=line("$")		" get new last line number...
	if (b:last == last) || ((last ==(b:last+1)) && getline(last)=='')	"nothing!!!
		if !b:samebuf
			unlet b:resultswindow
			bdelete!
		endif
	elseif last == (b:last+1)	" one more line
		let b:once=1
		call ResultsMap(1)
		call ResultsAct(last)
		let retval=1
	else					" more than one ... set up window
		normal 1zt
		" exe 'normal ' . b:last . 'j'
		set nomod
		let retval=2
		call ResultsMap(1)
	endif		
	
	exe "set ".mo."more ".te."terse"." ch=".ch." report=".rep
	return retval
endfunc

func! ResultsAct (lineno)
	if (a:lineno <= b:last) " in header
		return
	else
		let line=escape(getline(a:lineno),"'\"")
		let buffer=bufnr("")
		let action=b:action
      let origbuf = b:origbuf
		let dodelete = b:once && !b:samebuf

		"see if action is a function:
		if exists("*".action)
			" yes, so 'call'
			let act='call '.action."('".line."')"
			exe act
		elseif (action != '')
			" no, just exe
			exe action . ' ' . line
		endif
		if dodelete
			exe 'bdelete! '.buffer
         let buffer=bufnr("")
         " Reestablish alternate file
			exe 'buffer '.origbuf
			exe 'buffer '.buffer
		endif
	endif
endfunc

func! ResultsMap (on)
	if (a:on)
		map <cr> :call ResultsAct(line("."))<cr>
		map <2-LeftMouse> <cr>
		if (exists("b:maps") && b:maps != '')
			exe 'call' b:maps . '(1)'
		endif
	else
		if mapcheck("<cr>")!=''
			unmap <2-LeftMouse>
			unmap <cr>
		endif
		if (exists("b:maps") && b:maps != '')
			exe 'call' b:maps . '(0)'
		endif
	endif
endfunc

aug ResultsWindow
	au!
	au BufEnter * :if exists("b:resultswindow")|:call ResultsMap(1)|:endif
	au BufLeave * :if exists("b:resultswindow")|:call ResultsMap(0)|:endif
	au VimLeavePre * :call ResultsCleanup()
aug END

func! ResultsCleanup()
	let ix = 1
	while ix <= bufnr("$")
		" NOTE: 'getbufvar' is my enhancement, you need my patches!
		if exists("*getbufvar") && getbufvar(ix,'resultswindow')
			exe 'bd '.ix
		elseif match(bufname(ix), '-results-') >= 0
			exe 'bd '.ix
		endif
		let ix = ix + 1
	endwhile
endfunc

if 0

" we need 'g:vimlocal', so create it if not present
if !exists("g:vimlocal")
	let g:vimlocal = expand("~") . '/vimscripts'
endif
func! SourceIfReadable(filename)
	let retcode=1
	
	if filereadable(g:vimlocal . '/' . a:filename)
		exe 'so ' . g:vimlocal . '/' . a:filename
	elseif filereadable($VIM . '/' . a:filename)
		exe 'so ' . $VIM . '/' . a:filename
	elseif filereadable(a:filename)
		exe 'so ' . a:filename
	else
		let retcode=0
	endif
	return retcode
endfunc
command! -nargs=1 Source call SourceIfReadable(<q-args>)
func! Require(package)
	let shortname = fnamemodify(a:package, ':r')
	if !exists("g:".shortname.'_package_loaded')
		" loads 'package'.  First looks in 'vimscripts'
		if !SourceIfReadable(a:package)
			echo "Error!  Cannot load required package " . a:package
		endif
	endif
endfunc
command! -nargs=1 Require call Require(<q-args>)
SetIfBlank g:home  expand("~")

endif
