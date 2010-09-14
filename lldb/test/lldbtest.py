"""
LLDB module which provides the abstract base class of lldb test case.

The concrete subclass can override lldbtest.TesBase in order to inherit the
common behavior for unitest.TestCase.setUp/tearDown implemented in this file.

The subclass should override the attribute mydir in order for the python runtime
to locate the individual test cases when running as part of a large test suite
or when running each test case as a separate python invocation.

./dotest.py provides a test driver which sets up the environment to run the
entire test suite.  Users who want to run a test case on its own can specify the
LLDB_TEST and PYTHONPATH environment variables, for example:

$ export LLDB_TEST=$PWD
$ export PYTHONPATH=/Volumes/data/lldb/svn/trunk/build/Debug/LLDB.framework/Resources/Python:$LLDB_TEST:$LLDB_TEST/plugins
$ echo $LLDB_TEST
/Volumes/data/lldb/svn/trunk/test
$ echo $PYTHONPATH
/Volumes/data/lldb/svn/trunk/build/Debug/LLDB.framework/Resources/Python:/Volumes/data/lldb/svn/trunk/test:/Volumes/data/lldb/svn/trunk/test/plugins
$ python function_types/TestFunctionTypes.py
.
----------------------------------------------------------------------
Ran 1 test in 0.363s

OK
$ LLDB_COMMAND_TRACE=YES python array_types/TestArrayTypes.py

...

runCmd: breakpoint set -f main.c -l 42
output: Breakpoint created: 1: file ='main.c', line = 42, locations = 1

runCmd: run
output: Launching '/Volumes/data/lldb/svn/trunk/test/array_types/a.out'  (x86_64)

...

runCmd: frame variable strings
output: (char *[4]) strings = {
  (char *) strings[0] = 0x0000000100000f0c "Hello",
  (char *) strings[1] = 0x0000000100000f12 "Hola",
  (char *) strings[2] = 0x0000000100000f17 "Bonjour",
  (char *) strings[3] = 0x0000000100000f1f "Guten Tag"
}

runCmd: frame variable char_16
output: (char [16]) char_16 = {
  (char) char_16[0] = 'H',
  (char) char_16[1] = 'e',
  (char) char_16[2] = 'l',
  (char) char_16[3] = 'l',
  (char) char_16[4] = 'o',
  (char) char_16[5] = ' ',
  (char) char_16[6] = 'W',
  (char) char_16[7] = 'o',
  (char) char_16[8] = 'r',
  (char) char_16[9] = 'l',
  (char) char_16[10] = 'd',
  (char) char_16[11] = '\n',
  (char) char_16[12] = '\0',
  (char) char_16[13] = '\0',
  (char) char_16[14] = '\0',
  (char) char_16[15] = '\0'
}

runCmd: frame variable ushort_matrix
output: (unsigned short [2][3]) ushort_matrix = {
  (unsigned short [3]) ushort_matrix[0] = {
    (unsigned short) ushort_matrix[0][0] = 0x0001,
    (unsigned short) ushort_matrix[0][1] = 0x0002,
    (unsigned short) ushort_matrix[0][2] = 0x0003
  },
  (unsigned short [3]) ushort_matrix[1] = {
    (unsigned short) ushort_matrix[1][0] = 0x000b,
    (unsigned short) ushort_matrix[1][1] = 0x0016,
    (unsigned short) ushort_matrix[1][2] = 0x0021
  }
}

runCmd: frame variable long_6
output: (long [6]) long_6 = {
  (long) long_6[0] = 1,
  (long) long_6[1] = 2,
  (long) long_6[2] = 3,
  (long) long_6[3] = 4,
  (long) long_6[4] = 5,
  (long) long_6[5] = 6
}

.
----------------------------------------------------------------------
Ran 1 test in 0.349s

OK
$ 
"""

import os, sys
from subprocess import *
import time
import types
import unittest2
import lldb

if "LLDB_COMMAND_TRACE" in os.environ and os.environ["LLDB_COMMAND_TRACE"]=="YES":
    traceAlways = True
else:
    traceAlways = False


#
# Some commonly used assert messages.
#

CURRENT_EXECUTABLE_SET = "Current executable set successfully"

PROCESS_IS_VALID = "Process is valid"

PROCESS_KILLED = "Process is killed successfully"

RUN_SUCCEEDED = "Process is launched successfully"

RUN_COMPLETED = "Process exited successfully"

BREAKPOINT_CREATED = "Breakpoint created successfully"

BREAKPOINT_PENDING_CREATED = "Pending breakpoint created successfully"

BREAKPOINT_HIT_ONCE = "Breakpoint resolved with hit cout = 1"

STOPPED_DUE_TO_BREAKPOINT = "Process state is stopped due to breakpoint"

STOPPED_DUE_TO_STEP_IN = "Process state is stopped due to step in"

DATA_TYPES_DISPLAYED_CORRECTLY = "Data type(s) displayed correctly"

VALID_BREAKPOINT = "Got a valid breakpoint"

VALID_FILESPEC = "Got a valid filespec"

VALID_PROCESS = "Got a valid process"

VALID_TARGET = "Got a valid target"

VARIABLES_DISPLAYED_CORRECTLY = "Variable(s) displayed correctly"


#
# And a generic "Command '%s' returns successfully" message generator.
#
def CMD_MSG(command):
    return "Command '%s' returns successfully" % (command)

#
# Returns the enum from the input string.
#
def StopReasonEnum(string):
    if string == "Invalid":
        return 0
    elif string == "None":
        return 1
    elif string == "Trace":
        return 2
    elif string == "Breakpoint":
        return 3
    elif string == "Watchpoint":
        return 4
    elif string == "Signal":
        return 5
    elif string == "Exception":
        return 6
    elif string == "PlanComplete":
        return 7
    else:
        raise Exception("Unknown stopReason string")

#
# Returns the stopReason string given an enum.
#
def StopReasonString(enum):
    if enum == 0:
        return "Invalid"
    elif enum == 1:
        return "None"
    elif enum == 2:
        return "Trace"
    elif enum == 3:
        return "Breakpoint"
    elif enum == 4:
        return "Watchpoint"
    elif enum == 5:
        return "Signal"
    elif enum == 6:
        return "Exception"
    elif enum == 7:
        return "PlanComplete"
    else:
        raise Exception("Unknown stopReason enum")

#
# Returns an env variable array from the os.environ map object.
#
def EnvArray():
    return map(lambda k,v: k+"="+v, os.environ.keys(), os.environ.values())

# From 2.7's subprocess.check_output() convenience function.
def system(*popenargs, **kwargs):
    r"""Run command with arguments and return its output as a byte string.

    If the exit code was non-zero it raises a CalledProcessError.  The
    CalledProcessError object will have the return code in the returncode
    attribute and output in the output attribute.

    The arguments are the same as for the Popen constructor.  Example:

    >>> check_output(["ls", "-l", "/dev/null"])
    'crw-rw-rw- 1 root root 1, 3 Oct 18  2007 /dev/null\n'

    The stdout argument is not allowed as it is used internally.
    To capture standard error in the result, use stderr=STDOUT.

    >>> check_output(["/bin/sh", "-c",
    ...               "ls -l non_existent_file ; exit 0"],
    ...              stderr=STDOUT)
    'ls: non_existent_file: No such file or directory\n'
    """
    if 'stdout' in kwargs:
        raise ValueError('stdout argument not allowed, it will be overridden.')
    process = Popen(stdout=PIPE, *popenargs, **kwargs)
    output, error = process.communicate()
    retcode = process.poll()

    if traceAlways:
        if isinstance(popenargs, types.StringTypes):
            args = [popenargs]
        else:
            args = list(popenargs)
        print >> sys.stderr
        print >> sys.stderr, "os command:", args
        print >> sys.stderr, "stdout:", output
        print >> sys.stderr, "stderr:", error
        print >> sys.stderr, "retcode:", retcode
        print >> sys.stderr

    if retcode:
        cmd = kwargs.get("args")
        if cmd is None:
            cmd = popenargs[0]
        raise CalledProcessError(retcode, cmd, output=output)
    return output


class TestBase(unittest2.TestCase):
    """This LLDB abstract base class is meant to be subclassed."""

    # The concrete subclass should override this attribute.
    mydir = None

    # State pertaining to the inferior process, if any.
    # This reflects inferior process started through the command interface with
    # either the lldb "run" or "process launch" command.
    # See also self.runCmd().
    runStarted = False

    # Maximum allowed attempts when launching the inferior process.
    # Can be overridden by the LLDB_MAX_LAUNCH_COUNT environment variable.
    maxLaunchCount = 3;

    # Time to wait before the next launching attempt in second(s).
    # Can be overridden by the LLDB_TIME_WAIT environment variable.
    timeWait = 1.0;

    def setUp(self):
        #import traceback
        #traceback.print_stack()

        # Fail fast if 'mydir' attribute is not overridden.
        if not self.mydir or len(self.mydir) == 0:
            raise Exception("Subclasses must override the 'mydir' attribute.")
        # Save old working directory.
        self.oldcwd = os.getcwd()

        # Change current working directory if ${LLDB_TEST} is defined.
        # See also dotest.py which sets up ${LLDB_TEST}.
        if ("LLDB_TEST" in os.environ):
            os.chdir(os.path.join(os.environ["LLDB_TEST"], self.mydir));

        if "LLDB_MAX_LAUNCH_COUNT" in os.environ:
            self.maxLaunchCount = int(os.environ["LLDB_MAX_LAUNCH_COUNT"])

        if "LLDB_TIME_WAIT" in os.environ:
            self.timeWait = float(os.environ["LLDB_TIME_WAIT"])

        # Create the debugger instance if necessary.
        try:
            self.dbg = lldb.DBG
        except AttributeError:
            self.dbg = lldb.SBDebugger.Create()

        if not self.dbg.IsValid():
            raise Exception('Invalid debugger instance')

        # We want our debugger to be synchronous.
        self.dbg.SetAsync(False)

        # There is no process associated with the debugger as yet.
        # See also self.tearDown() where it checks whether self.process has a
        # valid reference and calls self.process.Kill() to kill the process.
        self.process = None

        # Retrieve the associated command interpreter instance.
        self.ci = self.dbg.GetCommandInterpreter()
        if not self.ci:
            raise Exception('Could not get the command interpreter')

        # And the result object.
        self.res = lldb.SBCommandReturnObject()

    def tearDown(self):
        #import traceback
        #traceback.print_stack()

        # Terminate the current process being debugged, if any.
        if self.runStarted:
            self.runCmd("process kill", PROCESS_KILLED, check=False)
        elif self.process and self.process.IsValid():
            rc = self.process.Kill()
            self.assertTrue(rc.Success(), PROCESS_KILLED)

        del self.dbg

        # Restore old working directory.
        os.chdir(self.oldcwd)

    def runCmd(self, cmd, msg=None, check=True, trace=False, setCookie=True):
        """
        Ask the command interpreter to handle the command and then check its
        return status.
        """
        # Fail fast if 'cmd' is not meaningful.
        if not cmd or len(cmd) == 0:
            raise Exception("Bad 'cmd' parameter encountered")

        trace = (True if traceAlways else trace)

        running = (cmd.startswith("run") or cmd.startswith("process launch"))

        for i in range(self.maxLaunchCount if running else 1):
            self.ci.HandleCommand(cmd, self.res)

            if trace:
                print >> sys.stderr, "runCmd:", cmd
                if self.res.Succeeded():
                    print >> sys.stderr, "output:", self.res.GetOutput()
                else:
                    print >> sys.stderr, self.res.GetError()

            if running:
                # For process launch, wait some time before possible next try.
                time.sleep(self.timeWait)

            if self.res.Succeeded():
                break

        # Modify runStarted only if "run" or "process launch" was encountered.
        if running:
            self.runStarted = running and setCookie

        if check:
            self.assertTrue(self.res.Succeeded(),
                            msg if msg else CMD_MSG(cmd))

    def expect(self, cmd, msg=None, startstr=None, substrs=None, trace=False):
        """
        Similar to runCmd; with additional expect style output matching ability.

        Ask the command interpreter to handle the command and then check its
        return status.  The 'msg' parameter specifies an informational assert
        message.  We expect the output from running the command to start with
        'startstr' and matches the substrings contained in 'substrs'.
        """
        trace = (True if traceAlways else trace)

        # First run the command.
        self.runCmd(cmd, trace = (True if trace else False))

        # Then compare the output against expected strings.
        output = self.res.GetOutput()
        matched = output.startswith(startstr) if startstr else True

        if startstr and trace:
            print >> sys.stderr, "Expecting start string:", startstr
            print >> sys.stderr, "Matched" if matched else "Not matched"
            print >> sys.stderr

        if substrs and matched:
            for str in substrs:
                matched = output.find(str) > 0
                if trace:
                    print >> sys.stderr, "Expecting sub string:", str
                    print >> sys.stderr, "Matched" if matched else "Not matched"
                if not matched:
                    break
            if trace:
                print >> sys.stderr

        self.assertTrue(matched, msg if msg else CMD_MSG(cmd))

    def invoke(self, obj, name, trace=False):
        """Use reflection to call a method dynamically with no argument."""
        trace = (True if traceAlways else trace)
        
        method = getattr(obj, name)
        import inspect
        self.assertTrue(inspect.ismethod(method),
                        name + "is a method name of object: " + str(obj))
        result = method()
        if trace:
            print str(method) + ":",  result
        return result

    def breakAfterLaunch(self, process, func, trace=False):
        """
        Perform some dancees after LaunchProcess() to break at func name.

        Return True if we can successfully break at the func name in due time.
        """
        trace = (True if traceAlways else trace)

        count = 0
        while True:
            # The stop reason of the thread should be breakpoint.
            thread = process.GetThreadAtIndex(0)
            SR = thread.GetStopReason()
            if trace:
                print >> sys.stderr, "StopReason =", StopReasonString(SR)

            if SR == StopReasonEnum("Breakpoint"):
                frame = thread.GetFrameAtIndex(0)
                name = frame.GetFunction().GetName()
                if trace:
                    print >> sys.stderr, "function =", name
                if (name == func):
                    # We got what we want; now break out of the loop.
                    return True

            # The inferior is in a transient state; continue the process.
            time.sleep(1.0)
            if trace:
                print >> sys.stderr, "Continuing the process:", process
            process.Continue()

            count = count + 1
            if count == 10:
                if trace:
                    print >> sys.stderr, "Reached 10 iterations, giving up..."
                # Enough iterations already, break out of the loop.
                return False

            # End of while loop.


    def buildDefault(self, compiler=None):
        """Platform specific way to build the default binaries."""
        module = __import__(sys.platform)
        if not module.buildDefault(compiler):
            raise Exception("Don't know how to build default binary")

    def buildDsym(self, compiler=None):
        """Platform specific way to build binaries with dsym info."""
        module = __import__(sys.platform)
        if not module.buildDsym(compiler):
            raise Exception("Don't know how to build binary with dsym")

    def buildDwarf(self, compiler=None):
        """Platform specific way to build binaries with dwarf maps."""
        module = __import__(sys.platform)
        if not module.buildDwarf(compiler):
            raise Exception("Don't know how to build binary with dwarf")

    def DebugSBValue(self, frame, val):
        """Debug print a SBValue object, if traceAlways is True."""
        if not traceAlways:
            return

        err = sys.stderr
        err.write(val.GetName() + ":\n")
        err.write('\t' + "TypeName    -> " + val.GetTypeName()          + '\n')
        err.write('\t' + "ByteSize    -> " + str(val.GetByteSize())     + '\n')
        err.write('\t' + "NumChildren -> " + str(val.GetNumChildren())  + '\n')
        err.write('\t' + "Value       -> " + str(val.GetValue(frame))   + '\n')
        err.write('\t' + "Summary     -> " + str(val.GetSummary(frame)) + '\n')
        err.write('\t' + "IsPtrType   -> " + str(val.TypeIsPtrType())   + '\n')
        err.write('\t' + "Location    -> " + val.GetLocation(frame)     + '\n')

