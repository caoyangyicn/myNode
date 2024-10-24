'use strict';

const my_console = internalBinding('my_console');

module.exports = {
    log: my_console.log
};
