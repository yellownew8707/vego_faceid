import sys
import gdb

# Update module path.
dir_ = '/home/huangxin/workspace/vego/out/arbutus-faceid/staging_dir/target/host/share/glib-2.0/gdb'
if not dir_ in sys.path:
    sys.path.insert(0, dir_)

from gobject import register
register (gdb.current_objfile ())
