(() => {
  const mapsMarker = 'data-breeze-maps-link';
  const adSelectors = [
    '#tads',
    '#tadsb',
    '[data-text-ad]',
    '[data-pla-slot-pos]',
    '[data-shopping-ads-container]'
  ];

  function removeAds() {
    for (const selector of adSelectors) {
      for (const node of document.querySelectorAll(selector)) node.remove();
    }
  }

  function getMapsUrl(query) {
    const url = new URL('https://www.google.com/maps/search/');
    url.searchParams.set('api', '1');
    url.searchParams.set('query', query);
    return url.href;
  }

  function findSearchNavigation() {
    for (const link of document.querySelectorAll('a[href]')) {
      const href = link.getAttribute('href') || '';
      if (!href.includes('udm=2') && !href.includes('tbm=isch')) continue;
      const navigation = link.closest('[role="navigation"]');
      if (navigation) return {navigation, link};
    }
    return null;
  }

  function hasMapsLink(navigation) {
    for (const link of navigation.querySelectorAll('a[href]')) {
      try {
        const url = new URL(link.href, location.href);
        if (url.pathname.startsWith('/maps')) return true;
      } catch {
      }
    }
    return false;
  }

  function stripTracking(root) {
    const nodes = [root, ...root.querySelectorAll('*')];
    for (const node of nodes) {
      node.removeAttribute('data-ved');
      node.removeAttribute('jsaction');
      node.removeAttribute('ping');
      node.removeAttribute('onmousedown');
    }
  }

  function ensureMapsLink() {
    const query = new URL(location.href).searchParams.get('q');
    if (!query) return;

    const current = document.querySelector(`[${mapsMarker}]`);
    if (current) {
      current.href = getMapsUrl(query);
      return;
    }

    const found = findSearchNavigation();
    if (!found || hasMapsLink(found.navigation)) return;

    let item = found.link;
    while (item.parentElement && item.parentElement !== found.navigation) {
      item = item.parentElement;
    }

    const clone = item.cloneNode(true);
    const link = clone.matches('a') ? clone : clone.querySelector('a');
    if (!link) return;

    stripTracking(clone);
    link.href = getMapsUrl(query);
    link.setAttribute(mapsMarker, '');
    link.setAttribute('aria-label', 'Maps');
    link.textContent = 'Maps';
    item.after(clone);
  }

  let scheduled = false;
  function refresh() {
    if (scheduled) return;
    scheduled = true;
    requestAnimationFrame(() => {
      scheduled = false;
      removeAds();
      ensureMapsLink();
    });
  }

  function start() {
    refresh();
    new MutationObserver(refresh).observe(document.documentElement, {
      childList: true,
      subtree: true
    });
  }

  if (document.documentElement) {
    start();
  } else {
    document.addEventListener('DOMContentLoaded', start, {once: true});
  }
})();
