###########
# project #
###########
project('ccdoc', 'c', version: '1.0.0')

cc = meson.get_compiler('c')

###########
# include #
###########

includeDir = include_directories('include')

###############
# source file #
###############

src = [ 'ef/str.c', 'ef/utf8.c', 'ef/memory.c', 'ef/math.c', 'ef/list.c', 'ef/vector.c', 'ef/delay.c', 'ef/fd.c', 'ef/file.c', 'ef/path.c', 'ef/dir.c', 'ef/rbhash.c', 'ef/hashalg.c' ]
src += [ 'ef/optex.c', 'ef/dstr.c', 'ef/config.c' ]
src += [ 'core/main.c', 'core/cparse.c', 'core/ccparse.c', 'core/ccdoc.c', 'core/ccdocHtml.c', 'core/ccdocREADME.c', 'core/ccdocWiki.c', 'core/ccdocMan.c' ]

##############
# data files #
##############

data =  [ 'template/template.html' ]
data += [ 'template/w3.css' ]
data += [ 'template/w3-theme-black.css' ]
data += [ 'template/scroll.css' ]
destdata = 'share/@0@'.format(meson.project_name())

#############
# man files #
#############

man =  [ 'doc/man/ccdoc.1' ]
man += [ 'doc/man/ccdoc_Comment Command.1' ]
man += [ 'doc/man/ccdoc_configure format.1' ]

##################
# compiler flags #
##################

# warnings
add_global_arguments('-Wall', language: 'c')
add_global_arguments('-Wextra', language: 'c')
add_global_arguments('-Wuninitialized', language: 'c')
add_global_arguments('-Winit-self', language: 'c')
add_global_arguments('-Wstrict-aliasing', language: 'c')
add_global_arguments('-Wstrict-overflow', language: 'c')
add_global_arguments('-Wfloat-equal', language: 'c')
add_global_arguments('-Wvla', language: 'c')

#reentrant malloc
add_global_arguments('-pthread', language: 'c')
add_global_link_arguments('-pthread', language:'c')

# standard
add_global_arguments('-std=gnu11', language: 'c')

# open mp
if get_option('openmp') > 0
  message('openmp enabled')
  add_global_arguments('-fopenmp', language:'c')
  add_global_link_arguments('-fopenmp', language:'c')
  add_global_arguments('-DOMP_ENABLE=1', language: 'c')
endif 

# gprof
if get_option('gprof') > 0
  add_global_arguments('-pg', language:'c')
  add_global_link_arguments('-pg', language:'c')
endif

# optimization
if( get_option('optimize') == 3 )
  message('agressive optimization enabled')
  add_global_arguments('-O3', language: 'c')
  add_global_arguments('-march=native', language: 'c')
  add_global_arguments('-mtune=native', language: 'c')
elif( get_option('optimize') == 2 )
  message('local optimization enabled')
  add_global_arguments('-O2', language: 'c')
  add_global_arguments('-march=native', language: 'c')
  add_global_arguments('-mtune=native', language: 'c')
elif( get_option('optimize') == 1 )
  message('generic optimization enabled')
  add_global_arguments('-O2', language: 'c')
else
  add_global_arguments('-g', language: 'c')
endif

if get_option('autovectorization') > 0
  if( get_option('optimize') < 1 )
	error('required option optimize > 0')
  endif
  message('vectorization enabled')
  add_global_arguments('-ftree-vectorize', language:'c')
  add_global_arguments('-DVECTORIZE=1', language:'c')
endif

##########################
# libraries dependencies #
##########################
libDeps  = [ cc.find_library('m', required : true) ]   # math
#libDeps += [ cc.find_library('dl', required : true) ] # dynamic load
libDeps += [ cc.find_library('rt', required : true) ]  # shared mem

#########################
# software dependencies #
#########################

#################
# Custom Target #
#################

#########
# debug #
#########

opt_debug = get_option('ebug')
if opt_debug > 0
  debugstr = 'debug enabled @0@'
  debugstr = debugstr.format(opt_debug)
  message(debugstr)
  debugstr = '-DDEBUG_ENABLE=@0@'
  debugstr = debugstr.format(opt_debug)
  add_global_arguments(debugstr, language: 'c')
  #add_global_arguments('-ftree-vectorizer-verbose=5', language:'c')
  #add_global_arguments('-fopt-info-loop-optimized', language:'c')
  #add_global_arguments('-fopt-info-vec-optimized', language:'c')
  #add_global_arguments('-fopt-info-vec-missed', language:'c')
endif

if get_option('olor') > 0
  message('color debug enabled')
  add_global_arguments('-DDEBUG_COLOR=1', language: 'c')
endif

if get_option('assert') > 0
  message('assertion enabled')
  add_global_arguments('-DASSERT_ENABLE=1', language: 'c')
endif

###########
# license #
###########

message('copyright vbextreme 2018')
message('released under GPLv3')

#########
# build #
#########

if( data.length() > 0 )
  message('install data on @0@'.format(destdata))
  install_data(data, install_dir: destdata)
endif

if( man.length() > 0 )
  message('install man files')
  install_man(man)
endif

executable(meson.project_name(), src, include_directories: includeDir, dependencies: libDeps, install: true)

