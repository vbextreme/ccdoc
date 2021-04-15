# ccdoc
this project was born because doxygen has never impressed me, I have always found it difficult for create html doc, man, readme and wiki.<br />

ccdoc only works with C, if you don't use C then you either change language or change software.<br />

ccdoc can generate HTML, README.md, WIKI and man page from your C Comment
<br />

Released under GPL v3<br />

<br />


## todo
write documentation for wrinting documentation

## How To

### Build and Install

```C

$ meson build
$ cd build
$ ninja

```



## Usage:
ccdoc search a cc.doc file where reading project info<br />

Short|Long|Required|Descript
-----|----|--------|--------
-c|--config|yes|change default config path, default is ./cc.doc
-h|--help|no|show this


## configure
you can change all layout of html page, *ccodcHTML for more info.<br />

<br />

can find format configuration on [configure format](https://github.com/vbextreme/ccdoc/wiki/configure%20format)<br />


```C

destdir_html   = ./doc/html                     // default value where stored .html
destdir_readme = ./doc/md                       // default value where stored README.md
destdir_wiki   = ./doc/wiki                     // default value where stored wiki files
destdir_readme = ./doc/man                      // default value where stored man files
meson_path     = ./meson.build                  // if you use meson, ccdoc read this file for get version and type of software
template_html  = /usr/share/ccdoc/template.html // default value
template_css   = /usr/share/ccdoc/template      // default value where read all css files to copy in destdir_html
src            = []                             // src is not default defined, required a vector of path where reading files
dump                                            // if exists dump ccdoc
html                                            // if exists build html doc
css                                             // if exists copy css
readme                                          // if exists build readme
wiki                                            // if exists build wiki
man                                             // if exists build man

```

// is a comment<br />

src contains files where search C Comment, you can add in this mode:<br />


```C

src = [ a.c, b.c]

```

or

```C

src = []
src += [ a.c ]
src += [ b.c ]

```

if string contains space you can quote or double quote.<br />

probably you want set destdir_readme to ./ and destsir_wiki to ./wiki.usernama for automatic overwrite README.md and Wiki pages.<br />

man pages generate name in this mode:<br />

for index page the name is same to filename, and other page are formatting to index_filename<br />



## C Comment
ccdoc add some special command to write in C comment, in this mode you can control output, only comment start with **-** is readed<br />

read [Comment Command](https://github.com/vbextreme/ccdoc/wiki/Comment%20Command) for more details


## News
**0.1**  ready, html,README,wiki,man<br />

**0.0**  begin<br />


