[ -n "$DISPLAY" ] && export TERM=xterm-color
SHELL=$BASH

if [ -n "$PS1" ] ;then
   [[ $0 != -* && $- != *l* ]] && . /etc/profile
   [[ $BASH_COMPLETION ]] || . /etc/bash_completion
   #. /etc/profile

   for p in ~/bin /usr/local/bin ;do
       [[ $PATH != *${p}:* ]] && PATH=${p}:$PATH ;done
   export SVN_EDITOR=vi
   HISTCONTROL=ignoredups
   IGNOREEOF=3
   HISTSIZE=2000
   HISTFILESIZE=50000

   __col() {
      [[ $TERM == xterm* || $TERM == linux ]] && echo -n '\[\e[3'${1}'m\]'
   }
   PS1="[$(__col 1)\h $(__col 2)\W$(__col 3)\$(exitrep)$(__col 9)]\\\$ "
   eval `dircolors ~/.dircolors`

   [ -f /etc/rc.d/functions ] && USECOLOR=yes source /etc/rc.d/functions

   alias mark='echo -e "\n\n\n\n      ${C_H2}---- `date` ----${C_CLEAR}\n\n\n\n"'
   alias l='   less'
   alias m='   mark'
   alias df='  df -k'
   alias ll='  ls -la'
   alias ls='  ls --color=auto'
   alias where='type -a'
   alias grep='grep --color=auto'
   alias svnhead="svnlog --limit=20"
   alias sort='LC_ALL=C sort'

   export SVN=svn+ssh://ampsvn/srv/ampemb
   svnlast()   { if [[ $# -ge 1 ]] ;then svnlog -p -l 25 "$@" ;else svnlog -p -a $USER -l 25 ;fi; }
   svnminfo()  { svn mergeinfo $SVN/trunk/embedded \
                               --show-revs ${@:-eligible} | \
                 xargs -i svnlog '-{}' svn+ssh://ampsvn/srv/ampemb/trunk/embedded; }

   svnst()     { svn st "$@" | grep -v  '^[X?]'; }
   hgst()      { hg  st "$@" | grep -v  '^[X?]'; }
   lth()       { ls -lat "$@" | head -20; }
   alias open='kfmclient exec'
   url()       { kfmclient openURL "$*"; }
   vimless()   { vim --cmd 'let no_plugin_maps = 1 | let load_less = 1' \
                     --cmd 'set readonly noswapfile' \
                      -c 'set mouse=a' \
                      -c 'runtime macros/less.vim' "${@:--}"; }
   p()         { "$@" | less;}
   v()         { if [[ $# -eq 1 && -e $1 ]] ;then vimless $1 ;else "$@" | vimless ;fi }
   vis()       { vi +set\ hlsearch $(which "$@"); }
   _txt()      { eval "file $*" | grep -w text | cut -d: -f1; }
   vif()       { vi -c set\ hlsearch "+/$*/" $(egrep -l "$*" $(_txt \*)); }
   vdiff()     { vi -g -f -d --cmd 'set columns=164' -c 'normal <C-W>=' "$@"; }
   mt()        { xterm +tb -e sh -c "while multitail $@; do : ;done"; }
   ToAURoot()  { tar zcf - $2 | command ssh $1 tar zxvf - -C /flash/root/; }
   Trace()     { amp-backtrace.lua "$@" | less +/'ERROR=>' --jump-target=.5; }
   psman()     { man -aW "$@" | xargs zcat | groff -man -Tps -P-pa4 \
                | psnup -pa4 -2 -d1 | less;}
   f()         { awk -v N="$*" 'BEGIN {split(N, Nr, / |,/)}
                 {for (n in Nr) {printf("%s%s", (n>1) ? " " : "", $Nr[n])}; print ""}'; }
   e()         { lua -e "print($*)"; }

   __h()
   {
      local -a C
      local -i I
      local N
      eval $(grep "$@" ~/.bash_history | uniq | command tail -10 | tac \
             | awk '{print "C[" I++ "]=\047" gensub("\047", "\047\\\\\047\047", "g", $0) "\047"}')
      if [[ ${#C[@]} > 0 ]] ;then
         for ((I=0; I<${#C[@]}; I++ )) ;do echo "$I: ${C[$I]}" > /dev/tty ;done
         read -n 1 -p "Select command: " N </dev/tty >/dev/tty
         if [[ $N = [0-9] && ${#C[$N]} > 0 ]] ;then
            echo "  ...  Running [$N]" >/dev/tty
            echo "${C[$N]}"
         else
            echo '' >/dev/tty
         fi
      fi
   }
   h() {
      local ___Very_UnL1leLy_7ar=$(__h "$@")
      history -s $___Very_UnL1leLy_7ar
      eval $___Very_UnL1leLy_7ar
   }
   alias hi="history|tail"

   case $TERM in
       sun-cmd) stln() { printf "\033]l %s \033" "$*"; } ; itit() { :; }
                ;;
       xterm*)  stln() { printf "\033]2;%s\007" "$*" ; }
                itit() { printf "\033]1;%s\007" "$*" ; }
                ;;
       *)       stln() { :; } ; itit() { :; }
       ;;
   esac
   HostnTty=`uname -n | tr '[a-z]' '[A-Z]'`:`tty | cut -c9-`
   Tty=`tty | cut -c10-`
   stdir() {
      local p=${PWD/$AMPROOT/@}
      if [[ $p = @/* ]] ;then Path=${p:2}; else Path=""; fi
      local p=${p/$HOME/\~}
      local V=''
      stln "-- $HostnTty ${TOOLCHAIN_PS1_LABEL/#tt/[tt${TARGET_PLATFORM/#V26/26}] }- $p --"
      itit "$Tty - $(sed 's/\([a-zA-Z]\)[a-zA-Z]*/\1/g' <<< ${AMPROOT##*/}) - ${p##*/}"
   }

   sshwrap() {
      local Cmd Host CmdU
      Cmd=$1 ; shift
      Host=$(command ssh -o 'ProxyCommand=echo %h >/dev/fd/9' -o ControlPath=none "$@" 9>&1 2>&-)
      CmdU=$(tr a-z A-Z <<<$Cmd)
      itit "$Tty - $CmdU $Host"
      command $Cmd "$@"
   }
   ssh()      { sshwrap ssh "$@"; }
   aussh()    { sshwrap aussh "$@"; }
   au-sshgw() { sshwrap au-sshgw "$@"; }
   tail()     { itit "$Tty - TAIL $@" ; command tail "$@"; }
   cu()       { itit "$Tty - CU $@"   ; command cu "$@"; }
   locate()
   {
      if [[ $# == 1 ]] ;then
         command locate -b "$@"
      else
         command locate "$@"
      fi
   }

   timerep() {
      local timeRep=$TIMESHOW ; [[ $timeRep ]] || timeRep=$TIMEREPORT
      [[ ! -n $timeRep ]] && return
      times > /tmp/.time$$
      local no timeStart cmd timeNow
      timeNow=$(< /tmp/.time$$)
      HISTTIMEFORMAT="%s "  history 1 > /tmp/.time$$
      read no timeStart cmd < /tmp/.time$$
      if [[ -n $_timeprev ]] ;then
         echo $_timeprev $timeNow | tr ms '  ' | awk '
         {
            usrPrev = $5*60 + $6
            sysPrev = $7*60 + $8
            usrCur = $13*60 + $14
            sysCur = $15*60 + $16
            total = (usrCur + sysCur) - (usrPrev + sysPrev)
            if (total >= timeRep) {
               printf("%.3fu %.3fs %d:%02d %.1f%%\n",
                      usrCur - usrPrev, sysCur - sysPrev,
                      duration / 60, duration % 60, total / duration * 100.0)
            }
         }' timeRep=$timeRep duration=$(( $(date +%s) - $timeStart ))
      fi
      _timeprev=$timeNow
      unset TIMESHOW
      rm -f /tmp/.time$$
   }
   TIMEREPORT=1
   tim() { TIMESHOW=0 ; "$@"; }

   exitrep() {
      declare -a P=( ${PIPESTATUS[@]} )
      local Last=$((${#P[*]} - 1))
      [[ ${P[$Last]} -ne 0 ]] && echo " E:${P[$Last]}"
   }

   profile_check() {
      local curtime=$(stat -c %y $HOME/.bashrc)
      if [[ $_profile_time && $_profile_time != $curtime ]] ;then
         source $HOME/.bashrc
      fi
      _profile_time=$curtime
   }

   PROMPT_COMMAND='history -a; stdir; timerep; profile_check'
   if [[ -d ~/.keychain && "$UID" -ne 0 ]] ;then
      #keychain --quiet ~/.ssh/id_dsa --timeout 1440  # 24 hours.
      keychain --quiet ~/.ssh/id_dsa
      . ~/.keychain/`uname -n`-sh
   fi

   if [[ -f /opt/toolchain/toolchain-setup ]] ;then
      tt()   { unset TOOLCHAIN_HOME ;. /opt/toolchain/toolchain-setup>/dev/null; }
      untt() { unset CROSS_COMPILE ARCH TOOLCHAIN_PS1_LABEL
               unalias make; }
      tt; untt
   fi

   amptree()  {
      if [ $# -eq 0 ] ;then echo $AMPROOT ; return ;fi
      if [ $1 == --completions ] ;then
         ( cd ~/amplex ; ls -1d */arm9 ) | cut -d/ -f1 | grep "^$3"
         return 
      fi
      local noCD=""
      if [[ $1 == --nocd ]] ;then noCD=1; shift; fi
      AMPROOT=~/amplex/$1
      CDPATH=""
      for d in ~  $AMPROOT/{,arm9,arm9/agentframework/{,agents,libs},arm9/apps/{,drivers},arm9/utils,arm9/drivers}  ~/{amplex,releases,src,abs,abs2} ;do
         CDPATH=$CDPATH:$d
      done
      export LUA_CPATH=";;$AMPROOT/arm9/agentframework/lua/?.so"
      if [[ ! -n $noCD ]] ;then
         echo $AMPROOT/arm9
         cd $AMPROOT/arm9
      fi
   }
   complete -C 'amptree --completions' amptree 
   amptree --nocd embedded
   complete -F _command -o filenames p v
   complete -c vis env where 
fi
# vim: set sw=3 sts=3 et:
