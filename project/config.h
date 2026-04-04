/**
 * @file config.h
 * @brief Application configuration and feature toggles.
 *
 * This header defines compile-time configuration options for controlling
 * various features and debug output in the application.
 */

#ifndef TRABALHO01_CONFIG_H
#define TRABALHO01_CONFIG_H

/**
 * @brief Controls whether to display detailed error messages.
 *
 * Set to 1 to enable error output, 0 to disable.
 * When disabled, no error messages will be printed.
 */
#define SHOW_ERRORS 1

/**
 * @brief Controls whether to show only generic error messages.
 *
 * Set to 1 to display only "Falha no processamento do arquivo." instead of
 * detailed error messages. Requires SHOW_ERRORS to be 1.
 * Set to 0 to display full detailed error messages.
 */
#define SIMPLE_ERRORS 1

/**
 * @brief Controls whether to run unit tests at startup.
 *
 * Set to 1 to automatically run all configured test suites on application start.
 * Set to 0 to skip tests and go directly to the main menu.
 */
#define RUN_TESTS 0

/**
 * @brief Controls whether to display the user menu.
 *
 * Set to 1 to show the menu with available options.
 * Set to 0 to hide menu output (useful for testing).
 */
#define SHOW_MENU 1

/**
 * @brief Controls whether to display input request prompts.
 *
 * Set to 1 to show prompts asking the user for input.
 * Set to 0 to suppress prompts (useful for batch processing).
 */
#define SHOW_INPUT_REQUEST 1

/**
 * @brief Controls whether to run continuous loop for interactive menu.
 *
 * Set to 1 to enable the continuous interactive menu loop for user input.
 * Set to 0 to disable the loop (useful for testing or batch processing).
 */
#define LOOP_RUN 1

#endif //TRABALHO01_CONFIG_H
