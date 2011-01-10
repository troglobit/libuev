# @(#)cshrc 1.11 89/11/29 SMI
umask 2
set path = ( ~/bin /usr/local/bin $path )
if ( $?prompt ) then
   # source /etc/CciCshEnv
   #foreach l ( "` env - PATH=/bin sh -c '. /etc/profile; env' | awk -f ~/.envtocsh`" )
   #   eval "$l"
   #end
   set HostnTty = `uname -n | tr '[a-z]' '[A-Z]'`
   set HostnTty = "$HostnTty":`tty | cut -c9-`
   set cdpath = ( /var/abs/* ~ )
   if (`tty` !~ /dev/pts/*) then
	   set term = `tset -Q -` >&/dev/null
   else if ( ! $?TERM) then
	   set term = vt100
   endif
   eval `dircolors -c ~/.dircolors`
   alias h history
   alias more less
   alias m more
   alias df 'df -k'
   alias ll 'ls -la'
   alias ls 'ls --color=auto'
   alias grep 'grep --color=auto'
   alias lth 'ls -lat \!* | head -20'
   alias open 'kfmclient exec'
   alias url 'kfmclient openURL "\!:*"'
   alias l  less
   alias p  '\!* | l'
   alias v  '\!* | vi - +set\ nomodified'
   alias vis   'vi +set\ hlsearch `/usr/bin/which \!*`'
   alias vif   'vi -c set\ hlsearch +/\!*/ `egrep -l \!* *`'
   alias prnt 'enscript -2 -r -j'
   alias vdiff  "vi -g -f -d --cmd 'set columns=164' -c 'normal <C-W>=' \!:*"
   alias mt 'xterm +tb -e sh -c "while multitail \!* ; do : ;done"'
   alias ToAURoot 'tar zcf - \!:2 | ssh \!:1 tar zxvf - -C /flash/root/'
   alias psman 'man -aW \!* | xargs zcat | groff -man -Tps -P-pa4 | psnup -pa4 -2 -d1 | less '
   alias autunnel 'ssh -L 22000:localhost:20532 -t ssh@gatekeeper ssh -p22\!:1 -L20532:localhost:22 root@localhost'
   alias aetunnel 'ssh -L 22000:localhost:20532 -t amplex@172.23.0.230 ssh -L20532:localhost:22 root@\!:1'

   if ( $TERM == sun-cmd ) then
      alias stln 'printf "\033]l %s \033\" \!*'
   else if ( $TERM =~ xterm* ) then
      alias stln 'printf "\033]2;%s\007\033]1;%s\007" \!* "$HostnTty"'
      alias xtitle 'printf "\033]2;%s\007\033]1;%s\007" \!:1 \!:1'
      printf "\033]1;%s\007" "$HostnTty"
   else
      alias stln 'echo "\!*" >/dev/null'
   endif
   alias stdir 'stln "-- $HostnTty - `/bin/pwd | sed s,^$HOME,~,` --"'
   alias cd    'set old="$cwd"; chdir \!:*; stdir'
   alias back  'set tmp="$cwd"; chdir "$old"; set old="$tmp" ; stdir'
   alias cb     back

   set history=200
   if ($?tcsh) then
      setenv SHELL /bin/sh
      set autolist
      set visiblebell
      set time = 2
		set histdup = prev
      set printexitvalue
		set ignoreeof = 2
      set echo_style = both
      set matchbeep=nomatch
		set color=ls-F
      unset autologout
      #bindkey -v
      #bindkey        vi-cmd-mode
      bindkey -e
      bindkey [1\;2~ complete-word-fwd
      bindkey [1\;6~ complete-word-back
      bindkey [1\;4~ complete-word-raw
      bindkey [1~    complete-word-fwd
      bindkey    ' '   magic-space
      bindkey -b ^W    backward-delete-word
      bindkey -b ^G    list-glob
      bindkey -b M-G    expand-glob
      bindkey -b M-P    dabbrev-expand
      bindkey -k up    history-search-backward
      bindkey -k down  history-search-forward
      bindkey -b ^A    spell-word
      bindkey -b ^P    dabbrev-expand
      bindkey -b ^N    normalize-path
      bindkey -b ^K    normalize-command

      alias _col ': \!:1'

      if ( $term =~ xterm* ) alias _col 'echo -n "%{^[[3\!:1m%}"'
      if ($uid > 0) then
         set prompt="`_col 1`%m`_col 9`.%h.`_col 2`%c`_col 9`> "
      else
         set prompt="`_col 1`%m`_col 9`.%h.`_col 2`%c`_col 9`# "
      endif

      # Esoteric
      complete find      'n/-fstype/(nfs ufs)/' 'n/-name/f/' \
                         'n/-type/(c b d f p l s)/' 'n/-user/u/' 'n/-exec/c/' \
                         'n/-ok/c/' 'n/-cpio/f/' 'n/-ncpio/f/' 'n/-newer/f/' \
                         'c/-/(fstype name perm prune type user nouser \
                              group nogroup size inum atime mtime ctime exec \
                              ok print ls cpio ncpio newer xdev depth)/' \
                         'n/*/d/'
      complete dd        'c/conv=/(ascii ebcdic ibm block lcase ucase swab \
                                   noerror notrunc sync)/' \
                         'c/if=/f/' 'c/of=/f/' \
                         'p/*/(if of ibs obs bs cbs files skip iseek \
                               oseek seek count conv)/=/'

      complete set       'p/1/s/'
      complete setenv    'p/1/e/'
      complete unset     'p/1/s/'
      complete unsetenv  'p/1/e/'
      complete vis       'p/1/c/'
      complete p         'p/1/c/'
      complete v         'p/1/c/'
      complete man       'p/1/c/'
      complete sudo      'p/1/c/'
      complete Su        'p/1/c/'
      complete alias     'p/1/a/'
      complete unalias   'p/1/a/'
      complete bindkey   'p/1/()' 'p/2/b/'
      complete complete  'p/1/c/'
      complete uncomplete 'p/1/C/'
      complete which     'p/*/c/'
      complete where     'p/*/c/'
      complete cd        'p/1/d/'
   endif
   if ( "$USER" != "root" ) then
      keychain --quiet ~/.ssh/id_dsa --timeout 1440  # 24 hours.
      source ~/.keychain/`uname -n`-csh
   endif
   alias tt 'source ~/.cshrc.tt'
   alias amptree 'setenv AMPROOT \!:1; set cdpath = ( ~ ~/{amplex,releases,src,abs,abs2} \!:1/{,arm9,arm9/agentframework/{,agents,libs},arm9/apps/{,drivers},arm9/utils,arm9/drivers} )'
   amptree ~/amplex/embedded_ampcom
   setenv SVN_EDITOR vi

   cd .
endif
