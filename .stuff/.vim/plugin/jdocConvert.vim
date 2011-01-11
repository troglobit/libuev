" Vim global plugin for javadoc comment wrangling
" Maintainer:	fm@amplex.dk

if exists("g:loaded_jdocConvert")
  " finish
endif
let g:loaded_jdocConvert = 1

let s:save_cpo = &cpo
set cpo&vim
 
let &cpo = s:save_cpo

if !hasmapto('<Plug>jdocConvertCompact')
  map <unique> <Leader>jc <Plug>jdocConvertCompact
endif
noremap <unique> <script> <Plug>jdocConvertCompact  <SID>ConvertCompact
if !hasmapto('<Plug>jdocConvertHere')
  map <unique> <Leader>jh <Plug>jdocConvertHere
endif
noremap <unique> <script> <Plug>jdocConvertHere  <SID>ConvertHere

function! s:JdocConvert(type, rng1, rng2) range
    let [rng1, rng2] = [a:rng1, a:rng2]
    if rng2 < rng1
        let [rng2,rng1] = [rng1,rng2]
    end
    let rng2 = max([rng1 + 3, rng2])
    let inp = getline(rng1, rng2)
    let out = []
    let txt = []
    let inside = -1
    let did_subst = 0
    let lead = ""
    let lno = rng2 - rng1
    for line in inp
        if line =~ '^\s*/\*\*'
            " Start jdoc
            let inside = 1
            let lead = substitute(line, '/\*\*.*$', "", "")
        elseif line =~ '^\s*\* ' && inside > 0
            " Inline jdoc
            call add(txt, substitute(line, '^\s*\* ', "", ""))
        elseif line =~ '^\s*\*/'
            " End jdoc
            let inside = 0
            if a:type == 1
                for l in txt 
                    call add(out, lead.'/// '.l)
                endfor
                let txt = []
                let did_subst = 1
            end
        elseif len(txt) > 0 && a:type == 2
            " Place text on first
            call add(out, line.' ///< '.txt[0])
            call remove(txt, 0)
            let did_subst = 1
            if len(txt) > 0
                let lead = repeat(" ", len(line)).' ///< '
                for l in txt 
                    call add(out, lead.l)
                endfor
            endif
            let txt = []
        else
            " Copy over
            call add(out, line)
        endif
        let lno += 1
    endfor
    if did_subst
        exe rng1.",".rng2 " delete"
        call append(rng1 - 1, out)
        exe rng1 - 1 + len(out) 
    endif
endfunction

function! s:CallConvert(bang, l1, l2)
    let type = a:bang == "!" ? 1 : 2
    call s:JdocConvert(type, a:l1, a:l2)
endfunction
 
nnoremap <SID>ConvertCompact  :call <SID>JdocConvert(1, line("."), line(".") + v:count1 - 1)<CR>
nnoremap <SID>ConvertHere     :call <SID>JdocConvert(2, line("."), line(".") + v:count1 - 1)<CR>
vnoremap <SID>ConvertCompact  :call <SID>JdocConvert(1, line("'<"), line("'>"))<CR>
vnoremap <SID>ConvertHere     :call <SID>JdocConvert(2, line("'<"), line("'>"))<CR>
command! -bang -range JdocConvert :call s:CallConvert("<bang>", <line1>, <line2>)

let &cpo = s:save_cpo
