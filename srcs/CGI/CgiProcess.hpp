/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: obajja <obajja@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/28 09:34:15 by mkerrien          #+#    #+#             */
/*   Updated: 2026/05/01 02:00:23 by obajja           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once





#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "Logging.hpp"
#include "WebServ.hpp"





typedef enum e_CgiStep
{
	CGI_EMPTY,
	CGI_RUNNING,
	CGI_DONE,
	CGI_ERROR
}	t_CgiStep;

typedef enum e_CgiError
{
	CGI_ERR_NONE,
	CGI_ERR_ALREADY_RUNNING,
	CGI_ERR_BAD_SPEC,
	CGI_ERR_BUILD_ARGV,
	CGI_ERR_BUILD_ENVP,
	CGI_ERR_PIPE_STDIN,
	CGI_ERR_PIPE_STDOUT,
	CGI_ERR_PIPE_EXEC,
	CGI_ERR_FCNTL_CLOEXEC,
	CGI_ERR_FCNTL_NONBLOCK_EXEC,
	CGI_ERR_FCNTL_NONBLOCK_STDIN,
	CGI_ERR_FCNTL_NONBLOCK_STDOUT,
	CGI_ERR_FORK,
	CGI_ERR_CHDIR,
	CGI_ERR_DUP2_STDIN,
	CGI_ERR_DUP2_STDOUT,
	CGI_ERR_EXECVE,
	CGI_ERR_READ_EXEC_PIPE,
	CGI_ERR_WRITE_STDIN,
	CGI_ERR_READ_STDOUT,
	CGI_ERR_WAITPID,
	CGI_ERR_CLOSE
}	t_CgiError;





typedef struct s_CgiException
{}								t_CgiException;

typedef struct s_CgiChildSetupError
{
	int							cgi_error;
	int							sys_errno;
}								t_CgiChildSetupError;

typedef struct s_CgiLaunchSpec
{
	std::string					program_path;
	std::string					working_directory;
	std::vector<std::string>	argv;
	std::vector<std::string>	env_lines;
	std::string					stdin_data;
}								t_CgiLaunchSpec;

typedef struct s_CgiRunState
{
	pid_t						pid;
	int							exec_fd_parent;
	int							stdin_fd_parent;
	int							stdout_fd_parent;
	size_t						stdin_sent;
	std::string					stdout_buffer;
	bool						exec_setup_checked;
	bool						stdin_closed;
	bool						stdout_eof;
	bool						spawned;
	int							exit_status;
	bool						child_status_collected;
	bool						child_exited_normally;
	bool						child_signaled;
	int							term_signal;
	int							raw_wait_status;
	int							child_setup_errno;
	bool						child_setup_failed_before_exec;
	t_CgiStep					step;
	t_CgiStep					last_step_before_error;
	t_CgiError					error;
}								t_CgiRunState;





class CgiProcess
{
public:
	CgiProcess();
	~CgiProcess();

	bool						spawn();
	bool						epollChildExit();
	bool						writeChildStdin();
	bool						readChildStdout();
	bool						closeChildStdin();

	bool						hasMinimalLaunchSpec() const;
	bool						hasError() const;
	bool						isComplete() const;

	void						addArg(const std::string &);
	void						addEnvLine(const std::string &);
	void						setProgramPath(const std::string &);
	void						setWorkingDirectory(const std::string &);
	void						setStdinData(const std::string &);

	t_CgiStep					getStep() const;
	const std::string			&getStdoutBuffer() const;

	int							getStdinFdParent() const;
	int							getStdoutFdParent() const;
	bool						isStdinClosed() const;
	bool						isStdoutEof() const;

	int							getExecFdParent() const;

	bool						checkExecFailurePipe();

	bool						terminateChildAndReap(void);



private:
	CgiProcess(const CgiProcess &src);
	CgiProcess &operator=(const CgiProcess &src);

	void						handleChildAfterFork(int (&)[2], int (&)[2],
								int (&)[2]);
	bool						handleParentAfterFork(int (&)[2], int (&)[2],
								int (&)[2]);

	bool						buildStringArray(char ***,
								const std::vector<std::string> &);
	void						freeStringArray(char **);
	bool						buildExecArrays();
	void						destroyExecArrays();

	void						resetLaunchSpec();
	void						resetRunState();

	bool						createStdinPipe(int (&)[2]);
	bool						createStdoutPipe(int (&)[2]);
	bool						setNonBlocking(int, t_CgiError);

	bool						applyChildWorkingDirectory();

	void						closeRunStateFds();
	void						closePipePair(int (&)[2]);
	void						closeIfOpen(int &);

	bool						isAlreadyRunning();
	bool						setError(t_CgiError);
	void						logError(t_CgiError);
	void						updateStep(t_CgiStep);
	void						updateCompletionState();
	void						decodeWaitStatus(int);

	const t_CgiLaunchSpec		&getLaunchSpec() const;
	const t_CgiRunState			&getRunState() const;
	char						**getArgvExec() const;
	char						**getEnvpExec() const;

	/* Handle execve() fail */
	bool						createExecFailurePipe(int (&)[2]);
	bool						setCloseOnExec(int);
	bool						writeChildSetupError(int, t_CgiError, int);
	bool						readChildSetupError(int, t_CgiChildSetupError &,
													bool &);





	t_CgiLaunchSpec				_launchSpec;
	t_CgiRunState				_runState;
	char						**_argvExec;
	char						**_envpExec;
};
