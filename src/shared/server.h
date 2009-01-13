/* $Id$ */

#ifndef SERVER_H
#define SERVER_H

#include "shared/sql.h"
#include "shared/network/core/os_abstraction.h"
#include "shared/network/core/core.h"

/**
 * @file server.h Shared server (master server/updater) related functionality
 */

class Server {
protected:
	SQL *sql;                              ///< SQL backend to read/write persistent data to
	bool stop_server;                      ///< Whether to stop or not

	/** Internal implementation of Run */
	virtual void RealRun() = 0;
public:
	/**
	 * Creates a new server
	 * @param sql          the SQL backend to read/write persistent data to
	 */
	Server(SQL *sql);

	/** The obvious destructor */
	virtual ~Server();

	/**
	 * Runs the application
	 * @param logfile          the file to send logs to when forked
	 * @param application_name the name of the application
	 * @param fork             whether to fork the application or not
	 */
	void Run(const char *logfile, const char *application_name, bool fork);

	/**
	 * Signal the server to stop at the first possible moment
	 */
	void Stop() { this->stop_server = true; }

	/**
	 * Returns the SQL backend we are currently using
	 * @return the SQL backend
	 */
	SQL *GetSQLBackend() { return this->sql; }
};

/**
 * Runs the application
 * @param argc             number of arguments coming from the console
 * @param argv             arguments coming from the console
 * @param hostname         variable to fill with the console supplied hostname
 * @param hostname_length  length of the hostname array
 * @param fork             variable to tell whether the arguments imply forking
 * @param application_name the name of the application
 */
void ParseCommandArguments(int argc, char *argv[], char *hostname, size_t hostname_length, bool *fork, const char *application_name);

/**
 * Multi os compatible sleep function
 * @param milliseconds to sleep
 */
void CSleep(int milliseconds);

#endif /* SERVER_H */
