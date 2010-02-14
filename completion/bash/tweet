__cltwitter_complete () {
  local cache="$HOME/.cltwitter_users.cache"
  local cur=${COMP_WORDS[COMP_CWORD]}
  local file_fresh=false
  COMPREPLY=()
  
  if [ ! -f ${cache} ]; then
    cltwitter-update-cache
    file_fresh=true
  fi
 
  if [[ "$cur" == \"@* ]]; then
    cur=${cur:2}
  elif [[ "$cur" == @* ]]; then
    cur=${cur:1}
  fi
  
  COMPREPLY=( $( compgen -W "`cat $cache`" -P @ -- $cur ) )
   
  if [ !$file_fresh ] && [ "`find ${cache} -mmin +15`" != "" ]; then
    (cltwitter-update-cache &) 2> /dev/null 
  fi
}

complete -F __cltwitter_complete -o default tweet
