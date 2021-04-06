# ccdoc v0.0
this project was born because doxygen has never impressed me, I have always found it difficult for create html doc, man, readme and wiki.</br>
ccdoc only works with C, if you don't use C then you either change language or change software.</br>
</br>
ccdoc is wip, really not use in production</br>
</br>
Released under GPL v3</br>

## TODO
write documentation for wrinting documentation

## How To

### Build and Install
```
$ meson build
$ cd build
$ ninja
```

### Test
```
$ mkdir ../doc
$ ./ccdoc -d -H -c ../template -t ../template/template.html -D ../doc ../test/ccdoc.h ../test/extra.h
```

### Template
for change html output edit template/template.html file</br>

## News
* **0.0**  begin

