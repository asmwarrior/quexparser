set d=%date:~0,10%
set t=%time:~0,8%
echo %d% %t% >>result.txt
quex -i cpp.qx --engine tiny_lexer --buffer-based --token-policy single --token-memory-management-by-user --token-id-offset 2 --token-prefix TKN_  --source-package   lexer >> result.txt