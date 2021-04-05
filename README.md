# ccdoc v0.0
simple doc generator for C</br>
</br>
Released under GPL v3</br>

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
$ ./ccdoc -d -H -t -c ../template ../template/template.html -D ../doc ../test/ccdoc.h ../test/extra.h
```

### Template
for change html output edit template/template.html file</br>

## News
* **0.0**  begin

