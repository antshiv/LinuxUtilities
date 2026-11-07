import unittest
from scripts.check_docs_policy import violations


class DocsPolicyTests(unittest.TestCase):
    def test_new_guide_rejected_even_with_html(self):
        self.assertTrue(violations([('A', 'docs/new.md'), ('A', 'docs/_pages/new.html')]))

    def test_existing_guide_requires_html(self):
        self.assertTrue(violations([('M', 'docs/guide.md')]))
        self.assertFalse(violations([('M', 'docs/guide.md'), ('M', 'docs/_pages/guide.html')]))

    def test_other_markdown_and_deletion_allowed(self):
        self.assertFalse(violations([('A', '.github/ISSUE_TEMPLATE/bug.md'), ('A', 'AGENTS.md'), ('D', 'docs/old.md')]))

    def test_deleted_html_does_not_satisfy_policy(self):
        self.assertTrue(violations([('M', 'README.md'), ('D', 'docs/_pages/index.html')]))
