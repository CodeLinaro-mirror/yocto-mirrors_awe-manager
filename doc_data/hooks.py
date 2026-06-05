import logging
import os
import re
from pathlib import Path

import git

import mkdocs.plugins

_logger = logging.getLogger()
_ABS_LINK_RE = re.compile(r'\]\(/((?!/)[^)]+)\)')


def on_config(config):
    site_url = os.environ.get('MKDOCS_SITE_URL', config.get('site_url', '/'))
    if not site_url.endswith('/'):
        site_url += '/'
    names = config['extra']['name']
    names['awe_host']   = f'[_Audio Processors_]({site_url}terminology/#audio-proc)'
    names['awe_sf']     = f'[_AWE Signal Flow_]({site_url}terminology/#signalflow)'
    names['awe_lib']    = f'[_AWE Core Library_]({site_url}terminology/#awecore)'
    names['controller'] = f'[_AWE-Controller_]({site_url}terminology/#awe-controller)'
    names['awctool']    = f'[_AWC-Tooling_]({site_url}terminology/#awctool)'
    names['awetc']      = f'[_AWC-TC_]({site_url}terminology/#awetc)'
    return config


def on_page_markdown(markdown, page, config, files):
    site_url = os.environ.get('MKDOCS_SITE_URL', config.get('site_url', ''))
    if not site_url:
        return markdown
    base = site_url.rstrip('/')
    return _ABS_LINK_RE.sub(lambda m: f']({base}/{m.group(1)})', markdown)


@mkdocs.plugins.event_priority(-100)
def on_env(env, config, files):
    fpath = Path(__file__).parent / ".."
    r = git.Repo(Path(__file__).parent / "..")  # need to be on GIT repo toplevel!
    hc = r.head.commit
    try:
        tag = r.git.describe(tags=False, dirty=True)
    except git.exc.GitCommandError:
        tag = "0.0.0"
        _logger.warn("Could not find a suitable GIT tag/description. Using 0.0.0 for tag!")
    env.globals["dspcgit"] = {
        "author": hc.author.author,
        "committer": hc.committer.name,
        "date": hc.committed_date,
        "tag": tag
        }
    config['extra']['dspcgit'] = env.globals["dspcgit"]
    if 'plugins' in config and 'with-pdf' in config['plugins']:
        pdf_output = config['plugins']['with-pdf'].config.data['output_path']
        config['plugins']['with-pdf'].config.data['output_path'] = pdf_output.format(env.globals["dspcgit"]["tag"])

    _logger.info(f"Entering on_env hook; added DSPCGIT data to ENV and CONFIG: {tag}")
    return env

