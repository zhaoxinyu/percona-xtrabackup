var common_stmts = require("common_statements");
var gr_memberships = require("gr_memberships");

var gr_node_host = "127.0.0.1";

if (mysqld.global.gr_id === undefined) {
  mysqld.global.gr_id = "uuid";
}

if (mysqld.global.gr_nodes === undefined) {
  mysqld.global.gr_nodes = [];
}

if (mysqld.global.md_query_count === undefined) {
  mysqld.global.md_query_count = 0;
}

if (mysqld.global.update_attributes_count === undefined) {
  mysqld.global.update_attributes_count = 0;
}

if (mysqld.global.router_version === undefined) {
  mysqld.global.router_version = "";
}

if (mysqld.global.router_rw_classic_port === undefined) {
  mysqld.global.router_rw_classic_port = "";
}

if (mysqld.global.router_ro_classic_port === undefined) {
  mysqld.global.router_ro_classic_port = "";
}

if (mysqld.global.router_rw_x_port === undefined) {
  mysqld.global.router_rw_x_port = "";
}

if (mysqld.global.router_ro_x_port === undefined) {
  mysqld.global.router_ro_x_port = "";
}

if (mysqld.global.router_metadata_user === undefined) {
  mysqld.global.router_metadata_user = "";
}

if (mysqld.global.perm_error_on_version_update === undefined) {
  mysqld.global.perm_error_on_version_update = 0;
}

if (mysqld.global.upgrade_in_progress === undefined) {
  mysqld.global.upgrade_in_progress = 0;
}

if (mysqld.global.queries_count === undefined) {
  mysqld.global.queries_count = 0;
}

if (mysqld.global.queries === undefined) {
  mysqld.global.queries = [];
}

if (mysqld.global.transaction_count === undefined) {
  mysqld.global.transaction_count = 0;
}

var nodes = function(host, port_and_state) {
  return port_and_state.map(function(current_value) {
    return [
      current_value[0], host, current_value[1], current_value[2],
      current_value[3]
    ];
  });
};


var group_replication_members_online =
    nodes(gr_node_host, mysqld.global.gr_nodes);

var metadata_version =
    (mysqld.global.upgrade_in_progress === 1) ? [0, 0, 0] : [1, 0, 2];
var options = {
  metadata_schema_version: metadata_version,
  group_replication_members: group_replication_members_online,
  innodb_cluster_instances: gr_memberships.cluster_nodes(
      mysqld.global.gr_node_host, mysqld.global.cluster_nodes),
  gr_id: mysqld.global.gr_id,
  router_version: mysqld.global.router_version,
  router_rw_classic_port: mysqld.global.router_rw_classic_port,
  router_ro_classic_port: mysqld.global.router_ro_classic_port,
  router_rw_x_port: mysqld.global.router_rw_x_port,
  router_ro_x_port: mysqld.global.router_ro_x_port,
  router_metadata_user: mysqld.global.router_metadata_user,
};

// prepare the responses for common statements
var common_responses = common_stmts.prepare_statement_responses(
    [
      "router_set_session_options",
      "router_set_gr_consistency_level",
      "select_port",
      "router_commit",
      "router_select_schema_version",
      "router_check_member_state",
      "router_select_members_count",
      "router_select_group_membership",
    ],
    options);

var router_update_attributes_strict_v1 =
    common_stmts.get("router_update_attributes_strict_v1", options);

var router_select_metadata =
    common_stmts.get("router_select_metadata", options);

var router_start_transaction =
    common_stmts.get("router_start_transaction", options);

({
  stmts: function(stmt) {
    // let's grab first queries for the verification
    if (mysqld.global.queries_count < 4) {
      var tmp = mysqld.global.queries;
      tmp.push(stmt)
      mysqld.global.queries = tmp;
      mysqld.global.queries_count++;
    }

    if (common_responses.hasOwnProperty(stmt)) {
      return common_responses[stmt];
    } else if (stmt === router_start_transaction.stmt) {
      mysqld.global.transaction_count++;
      return router_start_transaction;
    } else if (stmt === router_update_attributes_strict_v1.stmt) {
      mysqld.global.update_attributes_count++;
      if (mysqld.global.perm_error_on_version_update === 1) {
        return {
          error: {
            code: 1142,
            sql_state: "HY001",
            message:
                "UPDATE command denied to user 'user'@'localhost' for table 'routers'"
          }
        }
      } else
        return router_update_attributes_strict_v1;
    } else if (stmt === router_select_metadata.stmt) {
      mysqld.global.md_query_count++;
      return router_select_metadata;
    } else {
      return common_stmts.unknown_statement_response(stmt);
    }
  },
  notices: (function() {
    return mysqld.global.notices;
  })()
})
