# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_EXIT_CODES => 1, [<<'EOF']);
(my-dir-isdir) begin
(my-dir-isdir) mkdir "a"
(my-dir-isdir) create "b"
(my-dir-isdir) open "a"
(my-dir-isdir) open "b"
(my-dir-isdir) isdir "a"
(my-dir-isdir) isdir "b" (must fail)
(my-dir-isdir) end
EOF
pass;
