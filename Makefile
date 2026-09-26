#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : Makefile
# Description : Kritva Core build, test, lint, and development commands
#
# Component   : Core
# Module      : Development Infrastructure
# Layer       : Development Infrastructure
#
# Requirements: BUILD-001
# API         : GNU Make
#
# Author      : KritvaOS
# Created     : 26-09-2026
#==============================================================================

#------------------------------------------------------------------------------
# Project Configuration
#------------------------------------------------------------------------------

PROJECT_NAME := kritva-core

BUILD_DIR  := build
SRC_DIR    := src
TEST_DIR   := tests
SCRIPT_DIR := scripts

PYTHON := python3
CMAKE  := cmake

# Default build configuration.
# Override from command line:
#   make build BUILD_TYPE=Release
#------------------------------------------------------------------------------
BUILD_TYPE ?= Debug

#------------------------------------------------------------------------------
# Default Target
#------------------------------------------------------------------------------

.DEFAULT_GOAL := help

#------------------------------------------------------------------------------
# Help
#
# Targets containing "##" are automatically displayed by "make help".
#------------------------------------------------------------------------------

.PHONY: help

help: ## Show available development commands
	@echo ""
	@echo "Kritva Core - Development Commands"
	@echo "==================================="
	@echo ""
	@grep -E '^[a-zA-Z0-9_-]+:.*##' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS=":.*## "}; {printf "  %-20s %s\n", $$1, $$2}'
	@echo ""

#------------------------------------------------------------------------------
# Build
#------------------------------------------------------------------------------

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

.PHONY: header-check

header-check: ## Validate KritvaOS source headers
	@echo "[header-check] Validating KritvaOS source headers..."
	$(PYTHON) $(SCRIPT_DIR)/lint/check_source_headers.py \
		--mode tracked --strict

#------------------------------------------------------------------------------
# Code Formatting
#------------------------------------------------------------------------------

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

.PHONY: check

check: header-check format-check lint ## Run all local repository checks
	@echo ""
	@echo "[check] All repository checks passed."

#------------------------------------------------------------------------------
# Install
#------------------------------------------------------------------------------

.PHONY: install

install: build ## Install Kritva Core
	@echo "[install] Installing $(PROJECT_NAME)..."
	$(CMAKE) --install $(BUILD_DIR)

#------------------------------------------------------------------------------
# Information
#------------------------------------------------------------------------------

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
