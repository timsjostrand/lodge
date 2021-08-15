struct int_ops
{
	int lhs;
	int rhs;
};

static void cb_add(const struct int_ops *ops)
{
	printf("%d + %d = %d\n", ops->lhs, ops->rhs, (ops->lhs + ops->rhs));
}

static void cb_mult(const struct int_ops *ops)
{
	printf("%d * %d = %d\n", ops->lhs, ops->rhs, (ops->lhs * ops->rhs));
}

// callbacks test
#if 0
{
	struct lodge_callbacks callbacks = { 0 };
	lodge_callbacks_append(&callbacks, &cb_add, &(struct int_ops) { 1, 2 }, sizeof(struct int_ops));
	lodge_callbacks_append(&callbacks, &cb_mult, &(struct int_ops) { 1, 2 }, sizeof(struct int_ops));
	lodge_callbacks_run(&callbacks);
}
#endif
