/* standalone library for interchange between Binary Decision Diagram and
 * Sentential Decision Diagram setup instructions
*/

/* install BDD
 * https://github.com/utwente-fmt/sylvan
 * perform steps:
 *   mkdir build
 *   cd build
 *   cmake ..
 *   sudo make && make install
*/

/* install SDD
 * http://reasoning.cs.ucla.edu/sdd/
 * perform steps:
 *   scons
 *	 cp sddapi.h/usr/local/include/sddapi.h
 *	 cp build/libsdd.so/usr/local/lib/libsdd.so
 *   ldconfig
*/

/* install DD library
 * https://github.com/mrvanderzwart/DDinterchange
 * perform steps:
 *   mkdir build
 		 cd build
 		 cmake ..
 		 make

switch between BDDs and SDDs with the Boolean in <dd_exchange.h>
run your executable!
