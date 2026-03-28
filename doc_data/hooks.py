import logging
from pathlib import Path

import git

import mkdocs.plugins

_logger = logging.getLogger()

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

