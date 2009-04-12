__cltwitter_complete () {
  local cache="$HOME/.cltwitter_users.cache"
  local cur=${COMP_WORDS[COMP_CWORD]}
  COMPREPLY=()
  
  if [ ! -f ${cache} ] || [ "`find ${cache} -mmin +60`" != "" ]; then
    cltwitter-update-cache
  fi
 
  if [[ "$cur" == \"@* ]]; then
    cur=${cur:2}
  elif [[ "$cur" == @* ]]; then
    cur=${cur:1}
  fi
  
  COMPREPLY=( $( compgen -W "`cat $cache`" -P @ -- $cur ) )
 
}

complete -F __cltwitter_complete -o default tweet
