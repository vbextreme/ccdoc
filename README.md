# ccdoc
this project was born because doxygen has never impressed me, I have always found it difficult for create html doc, man, readme and wiki.<br />
ccdoc only works with C, if you don't use C then you either change language or change software.<br />
<br />
ccdoc is wip, really not use in production<br />
<br />
Released under GPL v3<br />
<br />
## TODO
write documentation for wrinting documentation
## How To
### Build and Install
```C

$ meson build
$ cd build
$ ninja
```

### Test
```C

TODO
```

### Template
for change html output edit template/template.html file

## Usage:
Short|Long|Required|Descript
-----|----|--------|--------
-c|--config|yes|chande default config path, default is cc.doc
-h|--help|no|show this
-h|--help|no|show this


### configure
read wiki/html/man fconfigure format for more documentation<br />
default ccdoc search cc.doc file and read configuration<br />
```C

destdir_html   = ./doc/html               // default value where stored .html
destdir_readme = ./doc/md                 // default value where stored README.md
template_html  = ./template/template.html // default value
template_css   = ./template               // default value where read css files to copy in destdir_html
src            = []                       // src is not default defined, required a vector of path where reading files
dump                                      // if exists dump ccdoc
html                                      // if exists build html doc
css                                       // if exists copy css
readme                                    // if exists build readme
```



## News
**0.2**  change option to config<br />
**0.1**  build README.md<br />
**0.0**  begin<br />

