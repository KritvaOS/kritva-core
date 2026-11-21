#==============================================================================
# Copyright (c) 2026 Kritva Robotics Private Limited.
# SPDX-License-Identifier: Apache-2.0
#
# File        : Makefile
# Description : Kritva Core build, test, lint, and development commands
#
# Component   : Core
# Module      : Development Infrastructure
# Layer       : Development Infrastructure
#
# Requirements: CORE-BUILD-001, CORE-REL-001
# API         : GNU Make
#
# Author      : KritvaOS
# Created     : 26-09-2026
#==============================================================================

#------------------------------------------------------------------------------
# Project Configuration
#------------------------------------------------------------------------------

# Kritva Makefile standard (kritva-program v2/planning/standards/MAKEFILE_STANDARD.md): `make` or `make help` lists the targets.
# A target is documented by `name: ## text` on its rule line; `##@ Name` starts a help group.
# An unimplemented gate exits 3 (NOT_IMPLEMENTED). It is never a pass.
.DEFAULT_GOAL := help
.PHONY: help

##@ Help
help: ## Show this list
	@awk 'BEGIN { FS = ":.*##"; printf "\nUsage: make <target> [VAR=value ...]\n" } \
	  /^##@/ { g = substr($$0, 5); next } \
	  /^[a-zA-Z0-9_.-]+:[^=].*##/ { if (g != "") { printf "\n%s\n", g; g = "" }; sub(/^ +/, "", $$2); printf "  %-24s %s\n", $$1, $$2 }' $(MAKEFILE_LIST)

PROJECT_NAME := kritva-core

BUILD_DIR  := build
SRC_DIR    := src
TEST_DIR   := tests
SCRIPT_DIR := scripts

PYTHON := python3
CMAKE  := cmake

# Code coverage configuration.
COVERAGE_BUILD_DIR := build-coverage
GCOVR := gcovr
COVERAGE_MIN_LINE ?= 0


# Default build configuration.
# Override from command line:
#   make build BUILD_TYPE=Release
#------------------------------------------------------------------------------
BUILD_TYPE ?= Debug

#------------------------------------------------------------------------------
# Default Target
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Help
#
# Targets containing "##" are automatically displayed by "make help".
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Build
#------------------------------------------------------------------------------
##@ Build

.PHONY: configure build rebuild clean

configure: ## Configure the CMake build
	@echo "[build] Configuring $(PROJECT_NAME)..."
	$(CMAKE) -S . -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure ## Build Kritva Core
	@echo "[build] Building $(PROJECT_NAME)..."
	$(CMAKE) --build $(BUILD_DIR) --parallel

rebuild: clean build ## Clean and rebuild Kritva Core

clean: ## Remove build artifacts
	@echo "[build] Cleaning build directory..."
	rm -rf $(BUILD_DIR)

#------------------------------------------------------------------------------
# Test
#------------------------------------------------------------------------------
##@ Test

.PHONY: test test-verbose

test: build ## Build and run tests
	@echo "[test] Running tests..."
	cd $(BUILD_DIR) && ctest --output-on-failure

test-verbose: build ## Build and run tests with verbose output
	@echo "[test] Running tests (verbose)..."
	cd $(BUILD_DIR) && ctest --output-on-failure --verbose

#------------------------------------------------------------------------------
# Source Header Check
#
# Validates the standardized KritvaOS source headers.
#
# The checker determines which tracked files require validation.
# Documentation, metadata, generated files, fixtures, and other excluded
# files are skipped according to the checker policy.
#------------------------------------------------------------------------------
##@ Source Header Check

.PHONY: header-check

header-check: ## Validate KritvaOS source headers
	@echo "[header-check] Validating KritvaOS source headers..."
	$(PYTHON) $(SCRIPT_DIR)/lint/check_source_headers.py \
		--mode tracked --strict

#------------------------------------------------------------------------------
# README standard check
#
# Structural check of README.md only (metadata, topics, relative links,
# placeholders). A PASS does not show that the content is accurate.
#------------------------------------------------------------------------------
##@ README standard check

.PHONY: check-readme

check-readme: ## Structural check of README.md against the Kritva README standard
	$(PYTHON) $(SCRIPT_DIR)/check_readme.py README.md

#------------------------------------------------------------------------------
# Requirements Traceability
#
# Audits REQUIREMENTS.md against headers, tests, and CMake registration.
#------------------------------------------------------------------------------
##@ Requirements Traceability

.PHONY: traceability-check

traceability-check: ## Audit requirements/API/test traceability
	@echo "[traceability] Auditing requirements traceability..."
	$(PYTHON) $(SCRIPT_DIR)/audit/check_traceability.py

#------------------------------------------------------------------------------
# API Documentation Audit
#
# Mechanical structure/reference audit of docs/api (never prose quality).
#------------------------------------------------------------------------------
##@ API Documentation Audit
.PHONY: api-docs-check

api-docs-check: ## Audit the Markdown API documentation (structure and references)
	@echo "[api-docs] Auditing API documentation..."
	$(PYTHON) $(SCRIPT_DIR)/audit/check_api_docs.py

#------------------------------------------------------------------------------
# Public API Inventory Audit (R1.0)
#
# Mechanical audit of docs/compatibility/API_INVENTORY.md against include/kritva/core/.
#------------------------------------------------------------------------------
##@ Public API Inventory Audit (R1.0)
.PHONY: api-inventory-check

api-inventory-check: ## Audit the public API inventory against the installed headers
	@echo "[api-inventory] Auditing public API inventory..."
	$(PYTHON) $(SCRIPT_DIR)/audit/check_api_inventory.py

#------------------------------------------------------------------------------
# Compatibility Policy Audit (R1.0)
#
# Mechanical structure/reference audit of docs/compatibility policy pages.
#------------------------------------------------------------------------------
##@ Compatibility Policy Audit (R1.0)
.PHONY: compat-policy-check

compat-policy-check: ## Audit the compatibility policy pages (structure and references)
	@echo "[compat-policy] Auditing compatibility policy..."
	$(PYTHON) $(SCRIPT_DIR)/audit/check_compat_policy.py

#------------------------------------------------------------------------------
# Public API Surface Snapshot Audit (R1.0)
#
# Detects any change to the public declaration surface against the reviewed snapshot.
#------------------------------------------------------------------------------
##@ Public API Surface Snapshot Audit (R1.0)
.PHONY: api-surface-check api-surface-update

api-surface-check: ## Compare the public API surface with tests/compat/api_surface.snapshot
	@echo "[api-surface] Auditing public API surface..."
	$(PYTHON) $(SCRIPT_DIR)/audit/check_api_surface.py

#------------------------------------------------------------------------------
# Security Documentation Audit (R1.0)
#
# Structure and classification consistency of docs/security and the R1.0 task records.
#------------------------------------------------------------------------------
##@ Security Documentation Audit (R1.0)
.PHONY: security-docs-check

security-docs-check: ## Audit the security documentation and security-impact classifications
	@echo "[security-docs] Auditing security documentation..."
	$(PYTHON) $(SCRIPT_DIR)/audit/check_security_docs.py

api-surface-update: ## Regenerate the API surface snapshot (a deliberate change; needs an evolution review)
	$(PYTHON) $(SCRIPT_DIR)/audit/check_api_surface.py --update

#------------------------------------------------------------------------------
# Code Formatting
#------------------------------------------------------------------------------
##@ Code Formatting

.PHONY: format format-check

format: ## Format source code
	@echo "[format] Formatting source files..."
	@echo "[format] TODO: configure clang-format"

format-check: ## Check source formatting
	@echo "[format] Checking source formatting..."
	@echo "[format] TODO: configure clang-format"

#------------------------------------------------------------------------------
# Static Analysis / Lint
#------------------------------------------------------------------------------
##@ Static Analysis / Lint

.PHONY: lint

lint: ## Run static analysis and lint checks
	@echo "[lint] Running static analysis..."
	@echo "[lint] TODO: configure clang-tidy / cppcheck"

#------------------------------------------------------------------------------
# Repository Validation
#
# "make check" is the main local validation command before creating a PR.
#
# Keep this broader than the Git pre-commit hook. The pre-commit hook should
# remain lightweight and run only the source-header checker.
#------------------------------------------------------------------------------
##@ Repository Validation

.PHONY: check

check: header-check traceability-check api-docs-check api-inventory-check compat-policy-check api-surface-check security-docs-check format-check lint ## Run all local repository checks
	@echo ""
	@echo "[check] The audit and header checks passed. format-check and lint are placeholders (nothing was checked), and this target does not run CTest."


#------------------------------------------------------------------------------
# Code Coverage
#
# Builds Kritva Core with GCC coverage instrumentation, runs the complete
# CTest suite, and generates a coverage report.
#
# Coverage is intentionally separate from the normal test target so that
# "make test" remains fast and unchanged.
#------------------------------------------------------------------------------
##@ Code Coverage

.PHONY: coverage coverage-check coverage-clean

coverage: ## Build, test, and generate code coverage report
	@echo "[coverage] Configuring coverage build..."
	$(CMAKE) -S . -B $(COVERAGE_BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="--coverage" \
		-DCMAKE_C_FLAGS="--coverage"

	@echo "[coverage] Building $(PROJECT_NAME)..."
	$(CMAKE) --build $(COVERAGE_BUILD_DIR) --parallel

	@echo "[coverage] Running tests..."
	cd $(COVERAGE_BUILD_DIR) && ctest --output-on-failure

	@echo "[coverage] Generating coverage report..."
	@mkdir -p coverage
	$(GCOVR) -r . \
		--exclude 'tests/.*' \
		--exclude 'build/.*' \
		--exclude 'build-coverage/.*' \
		--html-details coverage/coverage.html \
		--txt

	@echo "[coverage] Report: coverage.html"

coverage-check: coverage ## Run coverage and enforce minimum line coverage
	@echo "[coverage] Checking minimum line coverage..."
	$(GCOVR) -r . \
		--exclude 'tests/.*' \
		--exclude 'build/.*' \
		--exclude 'build-coverage/.*' \
		--fail-under-line $(COVERAGE_MIN_LINE)

coverage-clean: ## Remove coverage build and reports
	@echo "[coverage] Cleaning coverage artifacts..."
	rm -rf $(COVERAGE_BUILD_DIR)
	rm -f coverage.html coverage.html.*

#------------------------------------------------------------------------------
# Install
#------------------------------------------------------------------------------
##@ Install

.PHONY: install

install: build ## Install Kritva Core (PREFIX=<dir> overrides the install prefix)
	@echo "[install] Installing $(PROJECT_NAME)..."
	$(CMAKE) --install $(BUILD_DIR) $(if $(PREFIX),--prefix $(PREFIX))

#------------------------------------------------------------------------------
# Information
#------------------------------------------------------------------------------
##@ Information

.PHONY: info

info: ## Show build and project configuration
	@echo ""
	@echo "Kritva Core Configuration"
	@echo "========================="
	@echo "Project       : $(PROJECT_NAME)"
	@echo "Build type    : $(BUILD_TYPE)"
	@echo "Build dir     : $(BUILD_DIR)"
	@echo "Source dir    : $(SRC_DIR)"
	@echo "Test dir      : $(TEST_DIR)"
	@echo "Python        : $(PYTHON)"
	@echo "CMake         : $(CMAKE)"
	@echo ""

##@ Gates (unimplemented gates exit 3)
define not_implemented
	@echo "NOT_IMPLEMENTED: $(1) has no tests yet; this is not a pass"; exit 3
endef

.PHONY: test-unit
test-unit: ## Unit tests (NOT_IMPLEMENTED: exits 3)
	$(call not_implemented,test-unit)

.PHONY: test-integration
test-integration: ## Integration tests (NOT_IMPLEMENTED: exits 3)
	$(call not_implemented,test-integration)

.PHONY: test-regression
test-regression: ## Regression tests (NOT_IMPLEMENTED: exits 3)
	$(call not_implemented,test-regression)

.PHONY: acceptance
acceptance: ## Acceptance gate (NOT_IMPLEMENTED: exits 3)
	$(call not_implemented,acceptance)

.PHONY: docs-check
docs-check: ## Documentation link and structure check (NOT_IMPLEMENTED: exits 3)
	$(call not_implemented,docs-check)

##@ Preflight
# "make preflight" is the one command to run before pushing. It runs the checks CI runs on a pull request
# that can run on this machine, keeps going after a failure, prints a summary, fixes nothing and never
# fetches or installs anything. Not mirrored: secret-scan (gitleaks).
# PREFLIGHT_BASE is the branch the commits are compared with; run "git fetch origin" first.
# PREFLIGHT_SKIP_BUILD=1 leaves out the build and test step (the pre-push hook does this for pushes that change no code).
.PHONY: preflight preflight-commits readme-test commit-tests progress-check status-tests status-facts-check status-schema-check contract-pin-check hooks-install preflight-hook-on preflight-hook-off

PREFLIGHT_BASE ?= origin/main
PREFLIGHT_STEPS := check check-readme readme-test commit-tests preflight-commits contract-pin-check progress-check status-tests status-facts-check status-schema-check test

preflight: ## Run the CI checks locally and report every failure (PREFLIGHT_BASE=origin/main)
	@fail=""; \
	for step in $(if $(PREFLIGHT_SKIP_BUILD),$(filter-out test,$(PREFLIGHT_STEPS)),$(PREFLIGHT_STEPS)); do \
	  echo ""; echo "[preflight] >>> $$step"; \
	  if $(MAKE) --no-print-directory $$step; then :; else fail="$$fail $$step"; fi; \
	done; \
	echo ""; \
	$(if $(PREFLIGHT_SKIP_BUILD),echo "[preflight] Build and test step skipped (PREFLIGHT_SKIP_BUILD is set); CI still runs it.";) \
	if [ -n "$$fail" ]; then \
	  echo "[preflight] FAILED:$$fail"; \
	  echo "[preflight] Fix these, then run make preflight again. CI would reject this push."; \
	  exit 1; \
	fi; \
	echo "[preflight] All preflight checks passed. CI also runs secret-scan."

preflight-commits: ## Check the commits since PREFLIGHT_BASE: message rules, commit format
	@git rev-parse --verify --quiet "$(PREFLIGHT_BASE)^{commit}" >/dev/null || { echo "[preflight-commits] $(PREFLIGHT_BASE) not found. Run: git fetch origin"; exit 2; }
	@$(PYTHON) scripts/security/check_commit_messages.py --range "$(PREFLIGHT_BASE)..HEAD"
	@$(PYTHON) scripts/security/check_commit_format.py --range "$(PREFLIGHT_BASE)..HEAD"

commit-tests: ## Run the commit checker unit tests (same as the commit-message-check workflow)
	@$(PYTHON) -m unittest discover -s tests/commit_message_check

readme-test: ## Run the README checker unit tests (same as the readme-check workflow)
	@$(PYTHON) -m unittest discover -s tests/readme_check

progress-check: ## Check the progress report against its snapshot (same as the progress-check workflow)
	@$(PYTHON) scripts/progress_report.py --check

status-tests: ## Run the status generator tests (same as the status-report workflow)
	@$(PYTHON) -m unittest scripts/status/test_gen_status.py

status-facts-check: ## Check status facts against the planning records (same as the status-report workflow)
	@$(PYTHON) scripts/status/gen_status.py --check-facts

status-schema-check: ## Check the vendored schema hash and validate status/status.json (needs jsonschema 4.26.0, as CI installs)
	@cd status && sha256sum -c status.schema.json.sha256
	@$(PYTHON) -c 'import jsonschema' 2>/dev/null || { echo "[status-schema-check] jsonschema is not installed (CI uses 4.26.0). In a virtualenv: python3 -m pip install jsonschema==4.26.0"; exit 1; }
	@$(PYTHON) -c "import json, jsonschema; s = json.load(open('status/status.schema.json')); r = json.load(open('status/status.json')); jsonschema.Draft202012Validator(s).validate(r); assert r['schema_version'] == '2.0', 'unsupported schema_version'; print('status.json valid; tasks:', len(r['tasks']))"

contract-pin-check: ## Check the contract against its pin (same as the contract-pin-check workflow; needs full git history)
	@$(PYTHON) scripts/contract/check_contract_pin.py --verify-git

hooks-install: ## Enable the repository Git hooks (.githooks): make preflight runs before every git push
	git config core.hooksPath .githooks
	@echo "[hooks] core.hooksPath set to .githooks; pre-push runs make preflight."
	@echo "[hooks] Turn it off with: make preflight-hook-off. Skip it once with: git push --no-verify"

preflight-hook-on: ## Run "make preflight" before every git push (the default after hooks-install)
	git config kritva.preflightOnPush true
	@echo "[hooks] pre-push preflight is ON for this clone. It needs core.hooksPath (make hooks-install)."

preflight-hook-off: ## Turn off the pre-push preflight in this clone (CI still runs every check)
	git config kritva.preflightOnPush false
	@echo "[hooks] pre-push preflight is OFF for this clone."

##@ Git
.PHONY: git_sync
git_sync: ## Switch to main, fast-forward from origin, prune, delete merged local branches
	@git switch main
	@git pull --ff-only --prune
	@git branch --format='%(refname:short)' | grep -vx main | xargs -r git branch -d
	@git log --oneline -3
