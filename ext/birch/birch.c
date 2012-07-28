#include <ruby.h>

/* Notes to self:
 * - Unset can be called w/ one param.
 * - Add can be called w/ one param.
 * - Remove should be called only by ID.
 * - Link changed its API.
 */

/* Issues: GET value, id */

/*
 * Initialize the node with its value and id.
 * Setup containers for the children, features
 * and dependencies of this node.
 */
static VALUE birch_initialize(VALUE self, VALUE value, VALUE id)
{
	VALUE parent;
  VALUE children;
	VALUE children_hash;
	VALUE features;
	VALUE edges;
	
	parent = Qnil;
  children = rb_ary_new();
	children_hash = rb_hash_new();
	features = rb_hash_new();
	edges = rb_ary_new();
	
	rb_iv_set(self, "@value", value);
	rb_iv_set(self, "@id", id);

  rb_iv_set(self, "@parent", parent);
	rb_iv_set(self, "@children", children);
	rb_iv_set(self, "@children_hash", children_hash);
	rb_iv_set(self, "@features", features);
	rb_iv_set(self, "@edges", edges);
	
  return self;
}

/* Return the root of the tree. */
static VALUE birch_root(VALUE self) {
	
	VALUE ancestor;
	VALUE parent;
	VALUE tmp;
	
	parent = rb_iv_get(ancestor, "@parent");
	
	if (parent == Qnil) {
		return self;
	} else {
		ancestor = parent;
		while (ancestor != Qnil) {
			tmp = rb_iv_get(ancestor, "@parent");
			if (tmp == Qnil) {
				return ancestor;
			} else {
				ancestor = tmp;
			}
		}
		return Qnil;
	}
}

/* Add a child(ren) to this node. Return first child added. */
static VALUE birch_add(VALUE self, VALUE node_or_nodes) {
	
	long i;
	VALUE nodes;
	VALUE child;
	VALUE children;
	VALUE children_hash;
	VALUE child_id;
	
  children = rb_iv_get(self, "@children");
	children_hash = rb_iv_get(self, "@children_hash");
	
	if (CLASS_OF(node_or_nodes) != rb_cArray) {
		nodes = rb_ary_new();
		rb_funcall(nodes, rb_intern("push"), 1, node_or_nodes);
	} else {
		nodes = node_or_nodes;
	}
	
	for (i = 0; i < RARRAY_LEN(nodes); i++) {
		
		child = RARRAY_PTR(nodes)[i]; 
		child_id = rb_iv_get(child, "@id");
		
		rb_funcall(children, rb_intern("push"), 1, child);

		rb_funcall(children_hash, rb_intern("store"), 
			2, child_id, child
		);
		rb_iv_set(child, "@parent", self);
	}
  
	return RARRAY_PTR(nodes)[0];
	
}

/* Return the feature with the supplied name. */
static VALUE birch_get(VALUE self, VALUE feature) {
	if (rb_intern("id") == SYM2ID(feature)) {	
		return rb_iv_get(self, "@id");
	} else if (SYM2ID(feature) == rb_intern("value")) {
		return rb_iv_get(self, "@value");
	} else {
		return rb_hash_aref(rb_iv_get(self, "@features"), feature);
	}
}

/* Set the feature to the supplied value. */
static VALUE birch_set(VALUE self, VALUE feature, VALUE value) {
  return rb_hash_aset(rb_iv_get(self, "@features"), feature, value);
}

/* Unset a feature. */
static VALUE birch_unset(VALUE self, VALUE feature) {
	return rb_hash_delete(
		rb_iv_get(self, "@features"), 
		feature
	);
}

/* Return the total number of nodes in the subtree */
static VALUE birch_size(VALUE self) {
	

	VALUE children;
	int sum;
	long len;
	long i;
	
	children = rb_iv_get(self, "@children");
	len = RARRAY_LEN(children);
	sum = 0;
	
	for (i=0; i<len; i++) {
		sum += rb_funcall(
			RARRAY_PTR(children)[i], 
			rb_intern("size"), 0
		);
	}
	
	return INT2NUM(sum);
	
}

/* Iterate over each children of the node. */
static VALUE birch_each(VALUE self) {

	long i;
	VALUE children;
	children = rb_iv_get(self, "@children");
	
	RETURN_ENUMERATOR(children, 0, 0);
	
	for (i=0; i<RARRAY_LEN(children); i++) {
		rb_yield(RARRAY_PTR(children)[i]);
	}
	 
	return children;
	
}

static VALUE birch_find(VALUE self, VALUE id) {
	
	VALUE children_hash;
	VALUE children;
	VALUE result;
	long i;
	
	children_hash = rb_iv_get(self, "@children_hash");
	result = rb_hash_aref(children_hash, id);
	
	if (result == Qnil) {
		children = rb_iv_get(self, "@children");
		for (i=0; i < RARRAY_LEN(children); i++) {
			result = rb_funcall(
				RARRAY_PTR(children)[i], 
				rb_intern("find"), 1, id
			);
			if (result != Qnil) { return result; }
		}
		return Qnil;
	} else { return result; }
	
}

/* Boolean - is this node's subtree empty? */
static VALUE birch_is_leaf(VALUE self) {
	if (RARRAY_LEN(rb_iv_get(self, "@children")) == 0) {
		return Qtrue;
	} else {
		return Qfalse;
	}
}

/* Boolean - is this node parent-less? */
static VALUE birch_is_root(VALUE self) {
	if (rb_iv_get(self, "@parent") == Qnil) {
		return Qtrue;
	} else {
		return Qfalse;
	}
}

/* Boolean - does this node have children */
static VALUE birch_has_children(VALUE self) {
	if (RARRAY_LEN(rb_iv_get(self, "@children")) == 0) {
		return Qfalse;
	} else {
		return Qtrue;
	}
}

/* Boolean - does this node have children */
static VALUE birch_has_edges(VALUE self) {
	if (RARRAY_LEN(rb_iv_get(self, "@edges")) == 0) {
		return Qfalse;
	} else {
		return Qtrue;
	}
}

/* Boolean - does the node have a parent? */
static VALUE birch_has_parent(VALUE self) {
  if (rb_iv_get(self, "@parent") == Qnil) {
		return Qfalse;
	} else {
		return Qtrue;
	}
}

/* Boolean - does the node have the feature? */
static VALUE birch_has_feature(VALUE self, VALUE feature) {
	VALUE features;
	features = rb_iv_get(self, "@features");
  if (rb_hash_aref(features, feature) == Qnil) {
		return Qfalse;
	} else {
		return Qtrue;
	}
}

/* Boolean - does the node have features? */
static VALUE birch_has_features(VALUE self) {
	VALUE features;
	features = rb_iv_get(self, "@features");
  if (!RHASH(features)->ntbl) {
		return Qfalse;
	} else { return Qtrue; }
}


static VALUE birch_link(VALUE self, VALUE edge) {
	
	VALUE edges;
	edges = rb_iv_get(self, "@edges");
	
	rb_funcall(
		edges, rb_intern("push"), 1, edge
	);
	
}

/* Remove from parent and set as root */
static VALUE birch_set_as_root(VALUE self) {
	
	// Set the parent to be nil.
	rb_iv_set(self, "@parent", Qnil);
	
	// Return self.
	return self;
	
}

/* Removes a node by ID and returns it */
static VALUE birch_remove(VALUE self, VALUE id) {
	
	VALUE val;
	
	// Delete the node from the hash and retrieve it.
	val = rb_hash_delete(
		rb_iv_get(self, "@children_hash"), id
	);
	
	// Raise an exception if the value can't be found.
	if (val == Qundef) { 
		rb_raise(rb_eArgError, 
		"Given ID is not a children of this node.");
		return Qnil;
	}
	
	// Delete the node from the children array.
	rb_funcall(
		rb_iv_get(self, "@children"), 
		rb_intern("delete"), 1, val
	);
	
	// Set the node as a root (set its parent to nil).
	rb_funcall(
		val, rb_intern("set_as_root!"), 0
	);
		
	return val;

}

/* Remove all children and set them as root. */
static VALUE birch_remove_all(VALUE self) {

	VALUE children;
	long i;
	long children_len;
	
	children = rb_iv_get(self, "@children");
	
	for (i = 0; i < children_len; i++) {
		rb_funcall(
			RARRAY_PTR(children)[i], 
			rb_intern("set_as_root!"), 0
		);
	}
	
	rb_iv_set(self, "@children", rb_ary_new());
	rb_iv_set(self, "@children_hash", rb_hash_new());
	
	return self;
}

/* This class is a node for an N-ary tree data structure
 * with a unique identifier, text value, children, features
 * (annotations) and dependencies.
 *
 * This class was partly based on the 'rubytree' gem.
 * RubyTree is licensed under the BSD license and can
 * be found at http://rubytree.rubyforge.org/rdoc/.
 */
void Init_birch(void) {

	VALUE module = rb_define_module("Birch");
  VALUE birch_tree = rb_define_class_under(module, "Tree", rb_cObject);

	// Attribute accessors
	rb_attr(birch_tree, rb_intern("id"), 1, 1, 1);
	rb_attr(birch_tree, rb_intern("value"), 1, 1, 1);
	rb_attr(birch_tree, rb_intern("parent"), 1, 1, 1);
	rb_attr(birch_tree, rb_intern("features"), 1, 1, 1);
	
	// Attribute readers
	rb_attr(birch_tree, rb_intern("children"), 1, 0, 0);
	rb_attr(birch_tree, rb_intern("edges"), 1, 0, 0);
	
	// Methods
	rb_define_method(birch_tree, "initialize", birch_initialize, 2);
	rb_define_method(birch_tree, "root", birch_root, 0);
	rb_define_method(birch_tree, "<<", birch_add, 1);
	rb_define_method(birch_tree, "add", birch_add, 1);
	rb_define_method(birch_tree, "[]", birch_get, 1);
	rb_define_method(birch_tree, "get", birch_get, 1);
	rb_define_method(birch_tree, "[]=", birch_set, 2); 
  rb_define_method(birch_tree, "set", birch_set, 2); 
	rb_define_method(birch_tree, "unset", birch_unset, 1); 
	rb_define_method(birch_tree, "size", birch_size, 0);
	rb_define_method(birch_tree, "each", birch_each, 0);
	rb_define_method(birch_tree, "find", birch_find, 1);
	rb_define_method(birch_tree, "is_leaf?", birch_is_leaf, 0);
	rb_define_method(birch_tree, "is_root?", birch_is_root, 0);
	rb_define_method(birch_tree, "has_parent?", birch_has_parent, 0);
	rb_define_method(birch_tree, "has_children?", birch_has_children, 0);
	rb_define_method(birch_tree, "has_edges?", birch_has_edges, 0);
	rb_define_method(birch_tree, "has_feature?", birch_has_feature, 1);
	rb_define_method(birch_tree, "has_features?", birch_has_features, 0);
	rb_define_method(birch_tree, "has?", birch_has_feature, 1);
	rb_define_method(birch_tree, "link", birch_link, 1);
	rb_define_method(birch_tree, "set_as_root!", birch_set_as_root, 0);
	rb_define_method(birch_tree, "remove", birch_remove, 1);
	rb_define_method(birch_tree, "remove_all!", birch_remove_all, 0); 
	
}