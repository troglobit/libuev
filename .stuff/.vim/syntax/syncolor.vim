" Setup my own color preferences for syntax highlighting

if &background == "dark"
    hi Comment	term=bold ctermfg=Cyan guifg=#80a0ff
    hi Type	term=underline ctermfg=Magenta guifg=#ffa0a0
    hi Special	term=bold ctermfg=LightRed guifg=Orange
    hi Identifier term=underline cterm=bold ctermfg=Cyan guifg=#40ffff
    hi Statement term=bold ctermfg=Yellow guifg=#ffff60 gui=bold
    hi PreProc	term=underline ctermfg=LightBlue guifg=#ff80ff
    hi Constant	term=underline ctermfg=LightGreen guifg=#60ff60 gui=bold
    hi Ignore	ctermfg=black guifg=bg
else
    hi Comment	term=bold ctermfg=DarkBlue guifg=DarkBlue
    hi Type	term=underline ctermfg=Brown gui=NONE guifg=DarkRed
    hi Special	term=bold ctermfg=DarkMagenta guifg=DarkMagenta
    hi Identifier term=underline ctermfg=DarkCyan guifg=DarkCyan
    hi Statement term=bold ctermfg=DarkRed cterm=bold guifg=DarkRed gui=bold
    hi PreProc	term=underline ctermfg=DarkMagenta guifg=DarkMagenta
    hi Constant	term=underline ctermfg=DarkGreen guifg=DarkGreen gui=italic
    hi Ignore	ctermfg=white guifg=bg
    hi NonText	ctermfg=white
    hi! link NonText Ignore
endif
hi Error	term=reverse ctermbg=Red ctermfg=White guibg=Red guifg=White
hi Todo	term=standout ctermbg=Yellow ctermfg=Black guifg=Blue guibg=Yellow
hi SpellBad   ctermbg=NONE ctermfg=Red cterm=underline
hi SpellCap   ctermbg=NONE ctermfg=Blue cterm=underline
hi SpellRare  ctermbg=NONE ctermfg=Magenta cterm=underline
hi SpellLocal ctermbg=NONE ctermfg=Cyan cterm=underline
hi MatchParen ctermbg=NONE ctermfg=Cyan cterm=underline
"au! Syntax postscr source  /usr/local2/share/vim/syntax/postscr.vim

