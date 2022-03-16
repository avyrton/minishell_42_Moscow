#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <errno.h>
# include <dirent.h> //
# include <signal.h>
# include <fcntl.h>
# include <string.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <termios.h> //https://pubs.opengroup.org/onlinepubs/7908799/xbd/termios.html
# include <term.h>
# include <readline/readline.h>
# include <readline/history.h>

# define NAME_SHELL "minishell> "
# define SYMBOLS_SPECIAL "$\"\\`"
# define BUILTIN_FUNCION "cd echo env exit export unset pwd"
# define SUCCESS 0
# define ERROR (-1)

# define COMMAND 1
# define ARGUMENT 2
# define REDIR 3
# define INPUT 4
# define APPEND 5
# define PIPE 6
# define HEREDOC 7

typedef struct s_pipe
{
	int	flag;
	int	pipeout;
	int	pipein;
	int	roditel;
	int	count;
	int	closefd;
	int	insidepipe;
}	t_pipe;

typedef struct s_elements_arg
{
	int						type;
	char					*name;
	struct s_elements_arg	*next;
	struct s_elements_arg	*prev;
}	t_elements_arg;

typedef struct s_enpv
{
	char			*key;
	char			*value;
	struct s_enpv	*next;
}	t_envp;

typedef struct s_arg
{
	char			symb;
	struct s_arg	*next;
}	t_arg;

typedef struct s_signal
{
	int				sigint;
	int				sigquit;
	int				exit_status;
	pid_t			pid;
}	t_signal;

typedef struct s_mini
{
	t_envp			*envp;
	t_pipe			*pipe;
	t_elements_arg	*arg_elem;
	t_list			*work_history;
	int				arg_len;
	int				pipocunt;
	int				*pipelocs;
	char			*here_comand;
	int				fdin;
	int				fdout;
	int				fd_temp;
	int				in;
	int				out;
	int				is_quote_parse;
	int				no_exec;
	int				exit;
	int				ret;
	int				last;
	int				pid;
	int				heredoc;
	int				pipe_here;
	int				redir_flag;
}	t_mini;

extern t_signal	g_sig; // https://www.geeksforgeeks.org/understanding-extern-keyword-in-c/



//======== Functions_builtins.c

static void	init_funtions(int (*builtins_func[7])(t_mini *mini))
{
	builtins_func[0] = built_echo;
	builtins_func[1] = built_cd;
	builtins_func[2] = built_pwd;
	builtins_func[3] = built_export;
	builtins_func[4] = built_unset;
	builtins_func[5] = built_env;
	builtins_func[6] = built_exit;
}

int	builtins(t_minishell *mini)
{
	int (*builtins_func[7])(t_minishell	*mini);

	init_builtins(builtins_func);
	if (is_builtin(mini->arg_elem->name, "echo"))
		return (builtins_func[0](mini));
	else if (is_builtin(mini->arg_elem->name, "cd"))
		return (builtins_func[1](mini));
	else if (is_builtin(mini->arg_elem->name, "pwd"))
		return (builtins_func[2](mini));
	else if (is_builtin(mini->arg_elem->name, "export"))
		return (builtins_func[3](mini));
	else if (is_builtin(mini->arg_elem->name, "unset"))
		return (builtins_func[4](mini));
	else if (is_builtin(mini->arg_elem->name, "env"))
		return (builtins_func[5](mini));
	else if (is_builtin(mini->arg_elem->name, "exit"))
		return (builtins_func[6](mini));
	return (SUCCESS);
}



static void	print_cd_error(t_arg_item *args)
{
	ft_putstr_fd("cd: ", 2);
	if (args->next)
		ft_putstr_fd("string not in pwd: ", 2);
	else
	{
		ft_putstr_fd(strerror(errno), 2);
		ft_putstr_fd(": ", 2);
	}
	ft_putendl_fd(args->name, 2);
}

static int	set_oldpwd(t_envp *envp)
{
	char	cwd[255];

	if (!getcwd(cwd, 255))
		return (ERROR);
	if (is_env_exist(envp, "OLDPWD"))
	{
		ft_envp_update_node(envp, "OLDPWD", cwd);
	}
	else
		ft_envp_addback_node(&envp, ft_envp_new_node("OLDPWD", cwd));
	return (SUCCESS);
}
//======== built_echo.c

static int	check_option(t_arg_item **args, char *option_name)
{
	int	is_n;

	is_n = 0;
	while (*args && !ft_strncmp((*args)->name, option_name,
			ft_strlen(option_name)))
	{
		if (!ft_strncmp((*args)->name, option_name, ft_strlen(option_name))
			&& ft_strlen((*args)->name) == ft_strlen(option_name))
		{
			is_n++;
			(*args) = (*args)->next;
		}
		else
			break ;
	}
	if (is_n)
		return (1);
	return (0);
}

int	ft_echo(t_minishell *mini)
{
	t_arg_item	*args;
	int			n_opt;

	args = mini->arg_item->next;
	n_opt = check_option(&args, "-n");
	while (args && arg_item_count(mini->arg_item) > 1 && args->type < REDIR)
	{
		ft_putstr_fd(args->name, 1);
		if (args->next && ft_strlen(args->name) && (args->next->type < REDIR))
		{
			write(1, " ", 1);
		}
		args = args->next;
	}
	if (!n_opt)
		write(1, "\n", 1);
	return (SUCCESS);
}


//======== built_cd.c

static int	set_path(t_minishell *mini, t_envp *envp, int flag)
{
	int		result;
	t_envp	*envp_node;

	if (flag == 0)
	{
		envp_node = ft_copy_envp_node(ft_get_envp_node(envp, "OLDPWD"));
		if (!envp_node)
			return (item_not_found(mini, "cd", "OLDPWD not set"));
		set_oldpwd(envp);
		ft_putendl_fd(envp_node->value, 1);
	}
	else
	{
		set_oldpwd(envp);
		envp_node = ft_copy_envp_node(ft_get_envp_node(envp, "HOME"));
		if (!envp_node)
			return (item_not_found(mini, "cd", "HOME not set"));
	}
	result = chdir(envp_node->value);
	ft_free_str(envp_node->value);
	ft_free_str(envp_node->key);
	free(envp_node);
	envp_node = NULL;
	return (result);
}

int	ft_cd(t_minishell *mini)
{
	int			result;
	t_arg_item	*args;
	t_envp		*envp;

	envp = mini->envp;
	args = mini->arg_item->next;
	if (!args)
		return (set_path(mini, envp, 1));
	else if (!ft_strncmp(args->name, "-", 1))
		result = set_path(mini, envp, 0);
	else
	{
		set_oldpwd(envp);
		result = chdir(args->name);
		if (result < 0)
			result *= -1;
		if (result != 0)
			print_cd_error(args);
	}
	return (mini->ret = result);
}

//======== built_pwd.c

#include "minishell.h"

int	ft_pwd(t_minishell *mini)
{
	char	cwd[255];

	(void)mini;
	if (!getcwd(cwd, 255))
		return (1);
	ft_putendl_fd(getcwd(cwd, 255), 1);
	return (SUCCESS);
}

//======== built_export.c

static int	error_handle(t_arg_item *args, int valid_value)
{
	ft_putstr_fd("export: ", 2);
	ft_putstr_fd("'", 2);
	if (valid_value == -1 && args->next)
		ft_putstr_fd(args->next->name, 2);
	else
		ft_putstr_fd(args->name, 2);
	ft_putstr_fd("': ", 2);
	ft_putendl_fd("not a valid identifier", 2);
	return (ERROR);
}

static int	check_env(t_arg_item *args, char *env_name)
{
	int	i;

	i = 0;
	if (env_name[i] == '=')
		return (-1);
	if (env_name[i] != '_' && ft_isdigit(env_name[i]))
		return (0);
	while (env_name[i] && env_name[i] != '=')
	{
		if (!ft_isalpha(env_name[i]) && !ft_isdigit(env_name[i])
			&& env_name[i] != '_')
			return (0);
		i++;
	}
	if (args->next && ft_strncmp(args->next->name, "=", 1))
		return (0);
	return (1);
}

static void	add_env(t_minishell *mini, char *arg)
{
	int		i;
	char	*env_name;
	char	*env_val;

	i = 0;
	while (arg[i] != '=')
		i++;
	env_name = ft_substr(arg, 0, i);
	if (!env_name)
		return ;
	if (i == (int)ft_strlen(arg))
		env_val = ft_strnew(1);
	else
		env_val = ft_substr(arg, i + 1, ft_strlen(arg));
	if (!env_val)
		return ;
	if (!is_env_exist(mini->envp, env_name))
		ft_envp_addback_node(&mini->envp, ft_envp_new_node(env_name, env_val));
	else
		ft_envp_update_node(mini->envp, env_name, env_val);
	ft_free_str(env_name);
	ft_free_str(env_val);
}

int	ft_export(t_minishell *mini)
{
	int			is_valid_env;
	t_arg_item	*args;

	args = mini->arg_item->next;
	if (!args)
	{
		sort_envp_list(mini->envp);
		print_envp_list(mini->envp, "declare -x");
	}
	else
	{
		is_valid_env = check_env(args, args->name);
		if (is_valid_env == 1)
			add_env(mini, args->name);
		else
			return (error_handle(args, is_valid_env));
	}
	return (SUCCESS);
}

//======== unset.c

static void	delete_env(t_minishell *mini, char *var_name)
{
	t_envp	*node;

	if (is_env_exist(mini->envp, var_name))
	{
		node = ft_get_envp_node(mini->envp, var_name);
		if (node)
			ft_delete_envp_node(&mini->envp, node);
	}
}

int	ft_unset(t_minishell *mini)
{
	t_arg_item	*arg_item_tmp;

	arg_item_tmp = mini->arg_item->next;
	if (arg_item_tmp)
	{
		delete_env(mini, arg_item_tmp->name);
	}
	return (SUCCESS);
}


//======== built_env.c https://docs.microsoft.com/en-us/cpp/c-language/parsing-c-command-line-arguments?view=msvc-170

int	ft_env(t_minishell *mini)
{
	if (!is_env_exist(mini->envp, "PATH"))
	{
		item_not_found(mini, "env", "No such file or director\n");
		return (ERROR);
	}
	print_envp_list(mini->envp, NULL);
	return (SUCCESS);
}


//======== built_exit.c

static int	is_numeric(char *str)
{
	int	i;

	if (!str)
		return (0);
	i = 0;
	if (str[i] == '-')
		i++;
	while (str[i])
		if (!ft_isdigit(str[i++]))
			return (0);
	return (1);
}

void	message(t_arg_item *tmp)
{
	ft_putstr_fd("minishell: exit: ", 2);
	ft_putstr_fd(tmp->name, 2);
	ft_putendl_fd(": numeric argument required", 2);
}

int	ft_exit(t_minishell *mini)
{
	t_arg_item	*tmp;

	ft_putstr_fd("exit\n", 2);
	mini->exit = 1;
	tmp = mini->arg_item;
	if (tmp)
	{
		tmp = tmp->next;
		if (tmp && tmp->next)
		{
			mini->ret = 1;
			ft_putendl_fd("minishell: exit: too many arguments", 2);
		}
		else if (tmp && !is_numeric(tmp->name))
		{
			mini->ret = 255;
			message(tmp);
		}
		else if (tmp && tmp->name)
			mini->ret = ft_atoi(tmp->name);
		else
			mini->ret = 0;
	}
	return (mini->ret);
}

//======== built_utils.c

int	is_builtin(char *arg, char *builtin_name)
{
	int		i;
	char	**builtins;

	to_lower_case(arg);
	if (!arg || ft_strlen(arg) < 0)
		return (ERROR);
	if (builtin_name)
		return (is_equal_strs(arg, builtin_name));
	else
	{
		builtins = ft_split(BUILTIN_FUNC, ' ');
		if (!builtins)
			return (ERROR);
		i = 0;
		while (builtins[i])
		{
			if (is_equal_strs(arg, builtins[i++]))
			{
				ft_free_arr(builtins);
				return (1);
			}
		}
	}
	ft_free_arr(builtins);
	return (SUCCESS);
}


//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c



//======== .c
