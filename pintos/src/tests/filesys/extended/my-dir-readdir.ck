# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_EXIT_CODES => 1, [<<'EOF']);
(my-dir-readdir) begin
(my-dir-readdir) mkdir "a"
(my-dir-readdir) create "a/b"
(my-dir-readdir) open "a"
(my-dir-readdir) readdir "a"
(my-dir-readdir) name is "b"
(my-dir-readdir) readdir "a" (must fail)
(my-dir-readdir) end
EOF
pass;
