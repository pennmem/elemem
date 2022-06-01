import socket
from time import sleep, time, localtime, asctime
from random import randint
import logging
import sys
import csv
import os


class Tester:

    def __init__(self):
        self.HOST = "127.0.0.1"
        self.PORT = 8901
        self.initTime = asctime(localtime())
        self.conn = None
        self.addr = None

        try:
            os.makedirs('logs')
        except OSError:
            pass

        try:
            os.makedirs('csv')
        except OSError:
            pass

        targets = logging.StreamHandler(sys.stdout), logging.FileHandler(f'logs/{self.initTime}.txt')
        logging.basicConfig(format='%(message)s', level=logging.INFO, handlers=targets)

        """
        The tests array contains test sub-arrays with command tuples consisting of:
        (command, response time threshold (ms), expected response) """
        self.tests = []

        # Empty lists used to aggregate response times according to command
        self.responses = []
        self.stimResponses = []
        self.configResponses = []
        self.thetaConfigResponses = []

        self.testCSV = []  # used to construct csv output

        self.passes = 0
        self.fails = 0
        self.testPasses = 0
        self.testFails = 0

    # Sets new host
    def setHost(self, host):
        self.HOST = host

    # Sets new port
    def setPort(self, port):
        self.PORT = port

    # Adds multiple tests to the tests array
    def addTests(self, tests):
        for test in tests:
            self.addTest(test)

    # Adds a new test to the tests array
    def addTest(self, test):
        self.tests.append(test)

    # Adds randomly generated tests to the tests array
    def addGeneratedTests(self, numTests, numComms):
        for i in range(numTests):
            self.tests.append(self.configTestGen(numComms))

    # Random Config Command Generator - optional to include but useful for gathering statistics
    def configTestGen(self, n):
        testRand = [(b'R1999J', 4, b'SPREADY')]

        for i in range(n):
            comm = b'SPSTIMCONFIG,1,1,2,' + str(randint(1, 1000)).encode() + b',' + str(randint(4, 200)).encode() + b',' \
                   + str(randint(1, 1000)).encode()
            testRand.append((comm, 250, b'SPSTIMCONFIGDONE'))
            testRand.append((b'SPSTIMSTART', 4, b'SPSTIMSTARTDONE'))
            freq = randint(4, 200)
            comm = b'SPSTIMTHETACONFIG,1,1,2,' + str(randint(1, 1000)).encode() + b',' + str(freq).encode() + b',' + \
                   str(randint(1, freq)).encode() + b',' + str(randint(1, 99) / 100).encode()
            testRand.append((comm, 250, b'SPSTIMCONFIGDONE'))
            testRand.append((b'SPSTIMSTART', 4, b'SPSTIMSTARTDONE'))

        return testRand

    # Sorts response times according to command type
    def evalResponses(self, comm, r):
        if comm.split(b',')[0] == b'SPSTIMCONFIG':
            self.configResponses.append(r)
        elif comm.split(b',')[0] == b'SPSTIMTHETACONFIG':
            self.thetaConfigResponses.append(r)
        else:
            self.stimResponses.append(r)  # Includes subject code and start commands

    # Finds minimum, maximum, and average values of input lists
    def findStats(self, list):
        min = list[0]
        max = 0
        avg = 0

        for n in list:

            avg += n / len(list)

            if n < min:
                min = n

            if n > max:
                max = n

        logging.info(
            "Min: " + str(round(min, 3)) + " ms\t\t\t\tMax: " + str(round(max, 3)) + " ms\t\t\t\tAvg: " + str(round(avg, 3)) + " ms")

    # Writes csv file
    def writeCSV(self):
        with open(f'csv/{self.initTime}.csv', 'w', newline='') as csvfile:
            f = csv.writer(csvfile)
            f.writerows(self.testCSV)

    # Creates a line split on the console
    def lineSplit(self):
        logging.info("\n")
        logging.info("-" * 75)
        logging.info("\n")

    # Prints colored text to console
    def printResult(self, text, isPass, isBold):
        if isPass:
            print('\033[92m', end="")
        else:
            print('\033[91m', end="")
        if isBold:
            print('\033[1m', end="")

        logging.info(text)
        print('\033[0m', end="")

    # Runs all tests
    def runTests(self):

        # establishes server connection
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind((self.HOST, self.PORT))
            s.listen()

            for test in self.tests:  # iterates through all tests
                self.conn, self.addr = s.accept()

                with self.conn:
                    self.runCommands(test)

            sleep(0.001)  # millisecond delay prevents connection from failing - keep this in

    # Runs all input commands
    def runCommands(self, commands):

        testPass = True

        for command in commands:  # executes each command

            logging.info("Send: " + command[0].decode())
            self.conn.sendall(command[0] + b'\n')  # sends command to stimproc

            # Timer measures time between sent command and received data
            start = time()
            data = self.conn.recv(1024)
            end = time()
            responseTime = (end - start) * 1000  # calculates response time in ms rounded to 3 decimals

            logging.info("Recv: " + data.decode().strip())
            logging.info("Response Time: " + str(round(responseTime, 3)) + " ms")
            logging.info("Threshold: " + str(command[1]) + " ms")

            self.responses.append(responseTime)

            # Determines if response time measured is within threshold
            if responseTime <= command[1]:
                withinThreshold = True

            else:
                withinThreshold = False

            logging.info("Within Threshold: " + str(withinThreshold))

            # Determines if response matches the expected response
            if data.strip().split(b',')[0] == command[2]:
                expectedResponse = True
            else:
                expectedResponse = False

            logging.info("Expected Response: " + str(expectedResponse))

            # Determines if the test is a pass or failure - note that expected errors constitute passes
            if withinThreshold and expectedResponse:
                self.passes += 1
                self.evalResponses(command[0], responseTime)
                self.testCSV.append(["Pass", responseTime, command[0].decode(), data.strip().decode()])
                self.printResult("Pass", True, False)

            else:
                self.fails += 1
                testPass = False  # Test fails if a command fails
                self.testCSV.append(["Fail", responseTime, command[0].decode(), data.strip().decode()])
                self.printResult("Fail", False, False)

            self.lineSplit()

            sleep(0.25)  # delay between each command sent - this can be changed

        # Determines if the test passes overall - one command failure will result in a test failure
        if testPass:
            self.testPasses += 1
            self.printResult("Test Passed", True, True)

        else:
            self.testFails += 1
            self.printResult("Test Failed", False, True)

        self.lineSplit()

    def run(self):

        self.runTests()

        logging.info("Command Passes: " + str(self.passes) + "\t\t\tCommand Failures: " + str(self.fails))
        logging.info("Test Passes: " + str(self.testPasses) + "\t\t\t\tTest Failures: " + str(self.testFails))

        self.lineSplit()

        logging.info("All Responses:")
        self.findStats(self.responses)
        logging.info("\nStim Responses:")
        self.findStats(self.stimResponses)
        logging.info("\nConfig Responses:")
        self.findStats(self.configResponses)
        logging.info("\nThetaConfig Responses:")
        self.findStats(self.thetaConfigResponses)

        self.writeCSV()  # writes the csv file


def defaultTests(tester):

    tests = [

        [(b'R1999J', 4, b'SPREADY'), (b'SPSTIMCONFIG,1,1,2,500,200,750', 250, b'SPSTIMCONFIGDONE'),
         (b'SPSTIMSTART', 4, b'SPSTIMSTARTDONE'),
         (b'SPSTIMCONFIG,2,1,2,500,100,1000,3,4,500,200,1000', 250, b'SPSTIMCONFIGDONE'),
         (b'SPSTIMSTART', 4, b'SPSTIMSTARTDONE'),
         (b'SPSTIMTHETACONFIG,2,1,2,500,200,3,0.25,3,4,500,200,3,0.25', 250, b'SPSTIMCONFIGDONE'),
         (b'SPSTIMSTART', 4, b'SPSTIMSTARTDONE')],

        [(b'R1999T', 4, b'SPERROR')],

        [(b'R1999J', 4, b'SPREADY'), (b'NONSENSE', 4, b'SPERROR')],

        [(b'R1999J', 4, b'SPREADY'), (b'SPSTIMSTART', 4, b'SPSTIMSTARTERROR'),
         (b'SPSTIMCONFIG,1,1,2,500,200,2000', 250, b'SPSTIMCONFIGERROR'),
         (b'SPSTIMCONFIG', 250, b'SPSTIMCONFIGDONE'), (b'SPSTIMSTART', 4, b'SPSTIMSTARTERROR')],

        [(b'R1999J', 4, b'SPREADY'),
         (b'SPSTIMTHETACONFIG,2,1,2,500,200,3,0.25,3,4,500,200,5,0.25', 250, b'SPSTIMCONFIGERROR')]

    ]

    tester.addTests(tests)
    tester.addGeneratedTests(1, 10)


if __name__ == '__main__':
    tester = Tester()
    defaultTests(tester)
    tester.run()
