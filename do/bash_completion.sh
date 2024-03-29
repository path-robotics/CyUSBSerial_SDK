function complete_do_auto()
{
  COMPREPLY=()
  for d in do/"${2}"*; do
    case "${d}" in
      do/auto)
      ;;
      *)
        if [[ -f "${d}" ]] && [[ -x "${d}" ]]; then
          COMPREPLY+=("${d/do\//}")
        fi
      ;;
    esac
  done
}

complete -F complete_do_auto do/auto
