" vim: set ts=3 sw=3 et:
"""
""" %Z%%E% %U%, %I% %P%
""" (C) Copyright 2000 CCI-Europe. Author : fma
"""
""" Revision History """
""" End of Revisions """
"
" My .vimrc file
"
if has("win32")
   " Use CCI standard setup. This happens automatically on UNIX
   if filereadable($VIM . "/vimrc")
      source $VIM/vimrc
   endif
endif

syntax on
filetype plugin indent on
augroup filetype
autocmd BufEnter *.r set ft=xdefaults
augroup END

set tags=tags,../tags,/opt/toolchain/include/tags
set path=.,include,../include,/opt/toolchain/include
let CurPath = getcwd()
while strlen(CurPath) > 3
   if filereadable(CurPath."/tags")
      exe "set tags+=".CurPath."/tags"
   endif
   if strlen(glob(CurPath."/include"))
      exe "set path+=".CurPath."/include"
   endif
   if strlen(glob(CurPath."/*.h"))
      exe "set path+=".CurPath
   endif
   if !exists("g:GlimpsePath") && filereadable(CurPath."/.glimpse_index")
      let g:GlimpsePath = CurPath
      execute "set path+=".CurPath
   endif
   let CurPath = substitute(CurPath, '[/\\]\+[^/\\]*$', "", "")
endwhile

if has("win32")
   behave xterm
   set directory=C:/temp/preserve//   " Home of the swap files_
   if filereadable('C:\Programs\NT_SFU\Shell\grep.EXE')
      set grepprg=grep\ -n            " Use POSIX grep if available
   endif

   " Let VIM know about VisualC++ include files
   if exists("$include")
      let more = substitute($include, ';', ',', 'g')
      let more = substitute(more, ' ', '\\\\\\ ', 'g')
      execute "set path+=" . more
      let g:incdirs = g:incdirs.",".more

      let more = substitute($include, ';', '\\\\\\tags,', 'g')
      execute "set tags+=" . substitute(more, ' ', '\\\\\\ ', 'g') . '\\tags'
      unlet more
   endif

   if exists("$SCCSHOME")
      " Can not handle '/' to '\' conversion otherwise
      let $SCCSHOME = $SCCSHOME . "/"
   endif

   if has("gui_running")
      set guifont=Courier_New:h9
      " System menu and quick minimize
      map <M-Space> :simalt ~<CR>
      map <M-n> :simalt ~n<CR>
      " Size the GUI window. Delay positioning until window is created
      set lines=50
      set columns=82
      augroup AutoPos
      autocmd BufEnter * winp 200 50 | autocmd! AutoPos
      augroup END
   endif
else
   if (&term == "cygwin") " This seem to have some quirks, work around some
      set directory=/var/preserve/
      let &t_@7="\<Esc>[4~"   " Make <End> work
      let &t_kh="\<Esc>[1~"   " Make <Home> work
   else
      set directory=/var/preserve//
      if has("gui_running")
         set guifont=fixed
      else
         " Uncomment line below to use :emenu. Adds ~1 second to startup time
         " source $VIMRUNTIME/menu.vim

         " Make <End> work - Some options cannot be assigned to with 'let'
         exe "set t_@7=\<Esc>OF"

         " Must be _very_ slow to handle triple clicks
         set mousetime=1500
         if &ttymouse == "xterm2"
            " The xterm2 mode will flood us with messages
            set ttymouse=xterm
         endif

         " Save and restore the "shell" screen on enter and exit
         let &t_te = "\<Esc>[2J\<Esc>[?47l\<Esc>8"
         let &t_ti = "\<Esc>7\<Esc>[?47h"

         " Make <Del> mappings work
         exe "set t_kb=\x08"
         exe "set t_kD=\x7f"

         exe "set t_Co=16"

         " Make shifted cursor keys work.
         " For the necessary xmodmap commands, see :help hpterm
         map  <t_F3>   <S-Up>
         map! <t_F3>   <S-Up>
         map  <t_F6>   <S-Down>
         map! <t_F6>   <S-Down>
         map  <t_F8>   <S-Left>
         map! <t_F8>   <S-Left>
         map  <t_F9>   <S-Right>
         map! <t_F9>   <S-Right>
         " To make the shift-Tab <S-Tab> key work, also see :help suffixes
         cmap <Esc>[1~ <C-P>
         cmap <Esc>[1;2~ <C-P>
      endif
   endif
   if has("gui_running")
      let &guifont="Bitstream Vera Sans Mono 10"
   endif
endif

if has("gui_running")
    set title icon
    set titlestring=%F%(\ --\ %a%)
    set iconstring=%f
else
    set notitle noicon         " Do _not_ mess with title string of my xterm
endif

set showcmd                    " Show chars for command in progress on ruler
set exrc                       " Allow .vimrc in current dir
set hidden                     " Dont unload buffers BEWARE OF :q! and :qa! !!!
set backspace=indent,eol       " Can ^H/^W/^U across lines in insert
set sidescroll=5
set scrolloff=4
set textwidth=100
set whichwrap=h,l,<,>,[,]      " Do not backspace/space across line boundaries
set autoindent
set winheight=10               " Make new windows this high
set cmdheight=2                " status line area height - higher for quickfix
set laststatus=2               " always with statusline
set showmatch
set autowrite                  " write files back at ^Z, :make etc.
set mouse=nv                   " Use xterm mouse mode in insert/cmdline/prompt
set clipboard=autoselect       " Visual to/from clipboard
set fileformats=unix,dos,mac
set showbreak=________         " Show me where long lines break
set showfulltag                " Insert function prototype in ^X^]
set incsearch nohlsearch       " Search while typing, 'zx' toggles highlighting
set helpheight=100             " Maximize help windows
set shortmess=ato              " Brief messages to avoid 'Hit Return' prompts
set formatoptions=crqlo        " Comment handling / Dont break while typing
set history=100
set viminfo='20,\"500          " Keep history listings across sessions
set wildmenu                   " Show completion matches on statusline
set wildmode=list:longest,full
set complete=.,w,b,u,d         " The ,t,i from the default was too much, now ,d
set isfname-==                 " No = allows 'gf' in File=<Path> Constructs
set shiftwidth=4
set softtabstop=4
set expandtab
set spelllang=en_us
set equalprg=csb
set visualbell
set printfont=:h7.5
set printoptions=number:y,duplex:off,left:5mm,top:5mm,bottom:5mm,right:5mm

let c_gnu = 1
let c_no_curly_error = 1
"let &errorformat = 

" For running edit-compile-edit (quickfix)
nmap <Esc>n :cnext<CR>
nmap <Esc>l :clist<CR>
nmap <Esc>c :cc<CR>
nmap <Esc>p :cprevious<CR>
nmap <Esc>m :make<CR>

" Various utility keys
nmap (     :bprev<CR>
nmap )     :bnext<CR>
nmap F     :files<CR>
nmap Q     :bdelete<CR>
nmap !Q    :bdelete!<CR>
nmap [f    :bunload<CR>
nmap zx    :set invhlsearch<CR>
nmap zs    :set invspell<CR>
nmap zn    :set invnumber<CR>
nmap zf    :set invfoldenable<CR>
nmap [s    :exe "g/".@/."/p"<CR>


" These seem to be suitable for hitting all keys in mapping within timeout
set timeoutlen=500
set ttimeoutlen=50

" Make cursor keys jump out of insert. Your preference may differ
imap <Left>     <Esc>
imap <Right>    <Esc>
imap <Up>       <Esc>
imap <Down>     <Esc>
imap <S-Left>   <Esc>
imap <S-Right>  <Esc>
imap <S-Up>     <Esc>
imap <S-Down>   <Esc>

" Movement with "2x3" block navigation keys. Customize to your liking
nnoremap <PageUp>   <C-U>
nnoremap <PageDown> <C-D>
nnoremap <Home>     <C-Y>
nnoremap <xHome>    <C-Y>
nnoremap <End>      <C-E>
nmap <Insert>   [[z.
nmap <Del>      ]]z.
nmap <kDel>     ]]z.
nmap <S-Up>     {
nmap <S-Down>   }

" Insert some standard blobs
map <F2> :r $SCCSHOME/SccsHeaders/static.hdr<CR>

" Always delete to left of cursor
inoremap <Del> <C-H>
cnoremap <Del> <C-H>

" List multiple matches at CTRL-]
nmap <C-]>      :T <C-R><C-W><CR>
" Tag "preview" window
nmap <C-X><C-]>   :ptag <C-R><C-W><CR>
nmap <C-X><Right> :ptnext<CR>
nmap <C-X><Left>  :ptprevious<CR>
nmap <C-X><Down>  :ppop<CR>
nmap <C-X><Up>    :ptag<CR>
nmap <C-X>x       :pclose<CR>

" Some coloring. These are _my_ preferences
hi statusLine     term=bold,reverse cterm=NONE ctermbg=7 gui=NONE guibg=grey
hi statusLineNC   term=reverse      cterm=NONE ctermbg=8 gui=NONE guibg=darkgrey
hi NonText                                     ctermfg=lightgray
hi Visual                           cterm=Inverse ctermfg=grey ctermbg=black

if &term == "ansi" || &term == "console" || &term == "linux"
   set background=dark
   hi statusLine ctermfg=black
   hi statusLineNC ctermfg=black ctermbg=yellow
endif

" Use :T instead of :ta to see file names in ^D complete-lists
command! -complete=tag_listfiles -nargs=1 T tjump <args>|
                                           \call histadd("cmd", "T <args>")

" File type dependent settings
augroup Private
   " Use smartindent in these files.
   autocmd FileType tcl,postscr set nocindent smartindent|inoremap # X<BS>#
   " Use cindent for C files.
   autocmd FileType c,cpp   set cindent cinoptions=(0,w1,u0,:1,=2|inoremap # #
   " Autowrap while typing in other types of files
   autocmd FileType * set formatoptions-=t|set formatoptions+=t
   " NO autowrap while typing in source code files
   autocmd FileType tcl,postscr,c,cpp set formatoptions-=t | call SetPathAdd()

   " These are for CCI source files
   "autocmd BufRead * set ts=3 sts=3 sw=3
   " And _these_ are most sensible for VIM (ao. PD) source code
   "autocmd BufRead */src* set sts=4 sw=4 cinoptions&

   " Handle global (non bufferspecific) options
   autocmd BufEnter * call BufEnterGlobalOpts()
augroup end

function! SetPathAdd()
   let fpath = fnamemodify(expand("<afile>:p"), ":h")
   let SccsPath = $SCCSPATH
   let b:incdirs = ""
   while strlen(SccsPath) > 0
      let spc = match(SccsPath, " ")
      if spc >= 0
         let Part = strpart(SccsPath, 0, spc)
         let SccsPath = strpart(SccsPath,  spc + 1)
      else
         let Part = SccsPath
         let SccsPath = ""
      endif
      if strlen(Part) > 0
         let b:incdirs = b:incdirs.",".$SCCSHOME."/".$REL.".".Part."/".fpath
      endif
   endwhile
   if strlen(b:incdirs) > 1
      let b:incdirs = strpart(b:incdirs, 1)
   else
      unlet b:incdirs
   endif
endfunc

function! BufEnterGlobalOpts()
   " Avoid '#-in-1.-column' problem with cindent & smartindent
   if &filetype == 'c' || &filetype == 'cpp'
      inoremap # #
      " Dont highligh this as an error
      hi link cCommentStartError Comment
   else
      inoremap # X<BS>#
   endif

   " Set up path for private include files - w. help from SetPathAdd()
   if exists("g:incdirs")
      if exists("b:incdirs")
         let &path = ".,".b:incdirs.",".g:incdirs
      else
         let &path = ".,".g:incdirs
      endif
   endif

   let fpath = expand("<afile>:p")
   if fpath =~ '\f\/src\f' || fpath =~ '\f\/BUILD\f' || fpath =~ '\f-\d'
      setlocal patchmode=.orig            " Always save original file
   else
      setlocal patchmode&
   endif
endfunc


" Pick a status line, or craft one yourself
" statusline #1, two expressions -- Use a continuation line & quote SCCS keys
set statusline=%<%f%=\ %([%1*%M\%*%n%R\%Y%{VarExists(',GZ','b:zipflag')}
              \]%)\ %02c%V(%02B)C\ %3l/%LL\ %P
" statusline #2, zero expressions
"set statusline=%<%f%=\ %([%n%Xm%Xr%&,HL&h]%)\ %-19(%3l,%02c%'%o%)'%02Xb'
" statusline #3, the default one
"set statusline&

" Coloring of the modified flag
highlight User1 cterm=bold ctermfg=red ctermbg=gray gui=bold guifg=red guibg=gray

" Testing
"set title titlestring=%<%F%=%l/%L-%P titlelen=70
"set rulerformat=%l,%c%_%t-%P

function! VarExists(s,v)
   if exists(a:v)
      return a:s
   else
      return ""
   endif
endfunction

function! Show_g_CTRLG()
   let col   = col(".")
   let vcol  = virtcol(".")
   let line  = line(".")
   let lline = line("$")
   exe "normal $"
   let cole  = col(".")
   let vcole = virtcol(".")
   exe "normal " vcol . "|"

   let out_str = "Col " . col
   if col != vcol
      let out_str = out_str . "-" . vcol
   endif
   let out_str = out_str . " of " . cole
   if cole != vcole
      let out_str = out_str . "-" . vcole
   endif
   let out_str = out_str . "; Line " . line . " of " . lline
   let out_str = out_str . " (" . (line * 100 / lline) . "%)"
   let byte = line2byte(line) + col - 1
   let out_str = out_str . "; Char " . byte
   let lbyte = line2byte(lline) + strlen(getline(lline))
   let out_str = out_str . " of " . lbyte . " (" . (byte * 100 / lbyte) . "%)"
   echo out_str
endfunction
" This is _much_ faster than g<C-G> on large files. And it is more verbose
map gK :call Show_g_CTRLG()<CR>

function! Incr()
   if ! exists("g:Incr")
      let g:Incr = 0
   else 
      let g:Incr = g:Incr + 1
   endif
   return g:Incr
endfunc

if filereadable($HOME."/.vimrc.priv")
   source $HOME/.vimrc.priv
endif

if exists("load_less")
   set directory=
   set statusline=%3l/%LL\ %P\ %o/%{line2byte(line(\"$\")+1)-1}\ %=%<%f%a
   set cmdheight=1
endif

let loaded_explorer=1 " Don't want plugin/explorer.vim
" echo "DONE sourcing"

