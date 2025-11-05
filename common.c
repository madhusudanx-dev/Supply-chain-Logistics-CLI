#include "common.h"
#include <stdio.h>
#include <string.h>

double supplier_overall_score(const SupplierRatings *r) {
	// Weighted average (weights sum to 1)
	// Emphasize quality and reliability
	const double wQuality = 0.3;
	const double wReliability = 0.25;
	const double wDelivery = 0.2;
	const double wPrice = 0.15;
	const double wService = 0.1;
	return r->quality * wQuality + r->reliability * wReliability +
	       r->deliveryTime * wDelivery + r->price * wPrice + r->customerService * wService;
}

void trim_newline(char *s) {
	if (!s) return;
	size_t n = strlen(s);
	if (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[n-1] = '\0';
}

int safe_read_int() {
	int x;
	while (scanf("%d", &x) != 1) {
		int c;
		while ((c = getchar()) != '\n' && c != EOF) {}
		printf("Invalid input. Enter an integer: ");
	}
	// Clear remaining whitespace/newline after successful read
	int c;
	while ((c = getchar()) == ' ' || c == '\t') {}
	if (c != '\n' && c != EOF) {
		// Put back the character if it's not whitespace or newline
		ungetc(c, stdin);
	}
	return x;
}

double safe_read_double() {
	double x;
	while (scanf("%lf", &x) != 1) {
		int c;
		while ((c = getchar()) != '\n' && c != EOF) {}
		printf("Invalid input. Enter a number: ");
	}
	// Clear remaining whitespace/newline after successful read
	int c;
	while ((c = getchar()) == ' ' || c == '\t') {}
	if (c != '\n' && c != EOF) {
		// Put back the character if it's not whitespace or newline
		ungetc(c, stdin);
	}
	return x;
}


