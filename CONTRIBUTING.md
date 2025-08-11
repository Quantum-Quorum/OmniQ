# Contributing to OmniQ

Thank you for your interest in contributing to OmniQ! This document provides guidelines and information for contributors.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Development Setup](#development-setup)
3. [Code Style](#code-style)
4. [Testing](#testing)
5. [Pull Request Process](#pull-request-process)
6. [Reporting Issues](#reporting-issues)
7. [Code Review Guidelines](#code-review-guidelines)

## Getting Started

Before contributing, please:

1. Read the [README.md](README.md) to understand the project
2. Check existing [issues](https://github.com/Quantum-Quorum/OmniQ/issues) to see if your contribution is already being worked on
3. Join our [discussions](https://github.com/Quantum-Quorum/OmniQ/discussions) to connect with the community

## Development Setup

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16 or higher
- Python 3.8 or higher
- Git

### Building from Source

```bash
# Clone the repository
git clone https://github.com/Quantum-Quorum/OmniQ.git
cd OmniQ

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# Run tests
ctest --output-on-failure
```

### Python Development

```bash
# Install in development mode
cd omniq-python
pip install -e ".[dev,test,docs]"

# Run Python tests
pytest tests/
```

## Code Style

### C++ Code Style

We follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with some modifications:

- Use 4 spaces for indentation
- Maximum line length: 100 characters
- Use `snake_case` for functions and variables
- Use `PascalCase` for classes and structs
- Use `UPPER_CASE` for constants and macros

### Python Code Style

We follow [PEP 8](https://www.python.org/dev/peps/pep-0008/) with some modifications:

- Use 4 spaces for indentation
- Maximum line length: 88 characters (compatible with Black)
- Use `snake_case` for functions and variables
- Use `PascalCase` for classes
- Use `UPPER_CASE` for constants

### Formatting Tools

We use automated formatting tools to maintain consistent code style:

```bash
# C++ formatting (if clang-format is available)
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Python formatting
black omniq-python/
isort omniq-python/

# Linting
flake8 omniq-python/
mypy omniq-python/
```

## Testing

### C++ Tests

We use Google Test for C++ unit testing:

```bash
# Run all C++ tests
ctest --output-on-failure

# Run specific test
./test_core
./test_algorithms
```

### Python Tests

We use pytest for Python testing:

```bash
# Run all Python tests
pytest omniq-python/tests/

# Run with coverage
pytest --cov=omniq omniq-python/tests/

# Run specific test file
pytest omniq-python/tests/test_circuit.py
```

### Test Guidelines

- Write tests for all new functionality
- Aim for at least 80% code coverage
- Include edge cases and error conditions
- Use descriptive test names
- Group related tests in test classes

## Pull Request Process

1. **Fork the repository** and create a feature branch
2. **Make your changes** following the code style guidelines
3. **Add tests** for new functionality
4. **Update documentation** if needed
5. **Run all tests** to ensure nothing is broken
6. **Submit a pull request** with a clear description

### Pull Request Template

```markdown
## Description
Brief description of the changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Manual testing completed

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] No new warnings
```

## Reporting Issues

When reporting issues, please include:

1. **Clear description** of the problem
2. **Steps to reproduce** the issue
3. **Expected vs actual behavior**
4. **Environment information** (OS, compiler, Python version)
5. **Minimal example** if applicable

### Issue Template

```markdown
## Bug Description
Clear description of the bug

## Steps to Reproduce
1. Step 1
2. Step 2
3. Step 3

## Expected Behavior
What you expected to happen

## Actual Behavior
What actually happened

## Environment
- OS: [e.g., Ubuntu 20.04]
- Compiler: [e.g., GCC 9.4.0]
- Python: [e.g., 3.9.7]
- OmniQ version: [e.g., 0.1.0]

## Additional Information
Any other relevant information
```

## Code Review Guidelines

### For Reviewers

- Be constructive and respectful
- Focus on code quality and correctness
- Check for security issues
- Verify test coverage
- Ensure documentation is updated

### For Authors

- Respond to review comments promptly
- Make requested changes or explain why not
- Keep commits focused and well-described
- Address all review feedback

### Review Checklist

- [ ] Code is readable and well-documented
- [ ] Error handling is appropriate
- [ ] Security considerations addressed
- [ ] Performance is acceptable
- [ ] Tests are comprehensive
- [ ] Documentation is updated
- [ ] No breaking changes (unless intended)

## Getting Help

If you need help with contributing:

1. Check the [documentation](https://omniq.readthedocs.io/)
2. Search existing [issues](https://github.com/Quantum-Quorum/OmniQ/issues)
3. Start a [discussion](https://github.com/Quantum-Quorum/OmniQ/discussions)
4. Contact the maintainers at gouthamarcot@gmail.com

## License

By contributing to OmniQ, you agree that your contributions will be licensed under the Apache License 2.0.

Thank you for contributing to OmniQ! 