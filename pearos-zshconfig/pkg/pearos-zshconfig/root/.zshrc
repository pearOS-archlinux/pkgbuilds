HISTFILE=~/.zsh_history
HISTSIZE=10000
SAVEHIST=10000
setopt appendhistory
setopt sharehistory
setopt hist_ignore_dups

tty_dev=$(tty | sed -e "s:/dev/::")
last_login=$(last -1 -R $USER | head -1 | cut -c 23-)
echo "Last login: $last_login on ${tty_dev}"

PROMPT='%n@%m %1~ %f%# '

autoload -Uz compinit && compinit
zstyle ':completion:*' menu select

bindkey "^[[1;5D" backward-word
bindkey "^[[1;5C" forward-word
# Permite căutarea în istoric cu săgețile (Up/Down)
bindkey '^[[A' up-line-or-search
bindkey '^[[B' down-line-or-search

source /usr/share/zsh/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh

bindkey '^[[C' forward-char

alias ls='ls --color=auto -F'
alias ll='ls -lh'
alias dir='ls -F'
alias cls='clear'
alias la='ls -la'
