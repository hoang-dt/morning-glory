del compressed*
del run1.txt
del run2.txt

call run 2>run1.txt
mv compressed0 compressed0-old
mv compressed1 compressed1-old
mv compressed2 compressed2-old
mv compressed3 compressed3-old
mv compressed4 compressed4-old
mv compressed5 compressed5-old
mv compressed6 compressed6-old
mv expandedf expandedf-old
call run 2>run2.txt
