ps -ef | grep -v grep | grep "dycolor" | awk '{print $2}' | xargs kill
