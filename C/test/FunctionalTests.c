// Google Test 
// http://meekrosoft.wordpress.com/2009/11/09/unit-testing-c-code-with-the-googletest-framework/
// https://github.com/meekrosoft/helloc
// https://github.com/meekrosoft/fff
// or http://check.sourceforge.net/

// MOCK UART http://throwtheswitch.org/white-papers/when-bad-code-runs-green.html

// when execution fails bytes are left in the buffer, and the next time they are read they crash the python program, and cause unexpected behavior in the C library - advise cleaning the buffer promiscuously

// Bit Error Rate

// what if
    // transceiver is dead or powered down after opening the file handle
    // serial port is blocked?
    // memory allocation failed due to insufficient memory
    // wrong length reported in HE100 frame
    // line break or null byte in the data
    // simultanous send/receive
    // control of the port is changed, like if Netman crashes while fdin is open
    
// set a beacon after a certain period of inactivity

// Hardware Access:  In embedded systems we often have memory mapped hardware register access.  You most definitely don’t want to be dereferencing from random memory addresses in your tests.  A good antidote to this is to define a generic function to get the address for a given register.  You can then define a version of this function for testing purposes.

// Timing Problems.  That’s right, unit testing can’t magically simulate the runtime properties of your system.

// Interrupts.  This is a special case of the last point, but it is the same issue all developers come across when going multi-threaded.

// Bit-correct operations.  If you are running 24-bit code on a 32-bit architecture you will not see the exact same behavior for various overflow, underflow, bit-shifting and arithmetic operations.

// I can’t possibly test this!  Well, there are some classes of code that simply cannot be tested using the unit testing methodology.  In my experience however, it is an extreme minority of most code bases that this applies to.  The secret is to factor out impossible-to-test-code as much as possible so you don’t pollute the rest of the codebase.
