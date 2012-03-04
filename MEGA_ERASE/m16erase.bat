:avreal32 -ab -5 -p1 -o8000 +mega16 -e -fjtagen=1
:-w %1 -v -! -fboden=1,blev=1,sut=3,cksel=f,brst=1,bsiz=3,eesv=1,ckopt=1,jtagen=1,ocden=1,blb0=0,blb1=0 -l2
avreal32 -ab -5 -p1 -o8000 +mega16 -e -w %1 -v -! -fboden=1,blev=1,sut=3,cksel=f,brst=1,bsiz=3,eesv=1,ckopt=1,jtagen=0,ocden=1,blb0=0,blb1=0 -l2
@if ERRORLEVEL 1 pause
pause