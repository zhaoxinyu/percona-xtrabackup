#
# Basic test of InnoDB encryption support
#

. inc/keyring_common.sh
. inc/keyring_vault.sh
KEYRING_TYPE="component"
is_xtradb || skip_test "Keyring vault requires Percona Server"
keyring_vault_ping || skip_test "Keyring vault server is not avaliable"
# cleanup environment variables
MYSQLD_EXTRA_MY_CNF_OPTS="
innodb_redo_log_encrypt
innodb_undo_log_encrypt
binlog-encryption
log-bin
"
XB_EXTRA_MY_CNF_OPTS=

VAULT_MOUNT_VERSION=1
VAULT_CONFIG_VERSION="AUTO"
. inc/keyring_vault.sh

function cleanup_keyring() {
	keyring_vault_remove_all_keys
}

trap "keyring_vault_unmount" EXIT

test_do "ENCRYPTION='y'" "top-secret" ${KEYRING_TYPE}
test_do "ENCRYPTION='y' COMPRESSION='zlib'" "generate" ${KEYRING_TYPE}

keyring_vault_unmount
trap "" EXIT


# cleanup environment variables
MYSQLD_EXTRA_MY_CNF_OPTS="
innodb_redo_log_encrypt
innodb_undo_log_encrypt
binlog-encryption
log-bin
"
XB_EXTRA_MY_CNF_OPTS=
