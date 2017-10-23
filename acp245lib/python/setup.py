from distutils.core import setup
from distutils.extension import Extension
try:
    from Cython.Distutils import build_ext
    cython = True
except:
    cython = False

import commands

# Original code at http://code.activestate.com/recipes/502261/
def pkgconfig(*packages, **kw):
    flag_map = {'-I': 'include_dirs', '-L': 'library_dirs', '-l': 'libraries'}
    for token in commands.getoutput("pkg-config --libs --cflags %s" % ' '.join(packages)).split():
        if flag_map.has_key(token[:2]):
            kw.setdefault(flag_map.get(token[:2]), []).append(token[2:])
        else:
            kw.setdefault('extra_link_args', []).append(token)
    # remove duplicated
    for k, v in kw.iteritems():
        kw[k] = list(set(v))
    return kw

if cython:
    cmdclass = {'build_ext': build_ext}
    ext_modules = [
        Extension("acp245.pdu_gen",
                  [
                      "src/pdu_gen.pyx",
                  ],
                  **pkgconfig('e_libs-1.0', 'e_libs_buff-1.0',
                              'e_libs_utils-1.0',
                              include_dirs=['../src','src'],
                              library_dirs=['../src/.libs'],
                              libraries=['acp245'],
                              extra_compile_args=['-DE_ACP245_HAVE_E_LIBS']
                             )
                 ),
        Extension("acp245.log",
                  ["src/log.pyx"],
                  **pkgconfig('e_libs-1.0')
                 )
    ]
else:
    cmdclass = {}
    ext_modules = []

setup(
    name = "acp245",
    version = "1.6.0",
    description = "Python bindings for Edantech ACP245 library",
    author = "Santiago Aguiar",
    author_email = "santiago.aguiar@edantech.com",
    url = "http://www.edantech.com/",

    cmdclass = cmdclass,
    packages=['acp245', 'acp245.test'],
    package_dir={'acp245': 'src'},

    # ext_package does not work if a .pxd file it's present!
    # http://www.mail-archive.com/cython-dev@codespeak.net/msg05289.html
    # http://www.mailinglistarchive.com/sage-support@googlegroups.com/msg03083.html
    #ext_package='acp245',
    ext_modules = ext_modules,
    classifiers = [
        'License :: (c) Edantech. All rights reserved.'
    ]
)
